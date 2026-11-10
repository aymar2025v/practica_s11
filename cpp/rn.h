// ============================================================================
//  rn.h  -  Arbol Rojo-Negro generico (C++17)
//  SIS210 - Algoritmos y Estructuras de Datos | Practica Semana 11
//
//  IDEA GENERAL
//  ------------
//  Un arbol Rojo-Negro (RN) es un BST que guarda un bit extra por nodo (el
//  color) y obliga a cumplir 5 propiedades:
//    P1. Cada nodo es ROJO o NEGRO.
//    P2. La raiz es NEGRA.
//    P3. Las hojas NIL (nullptr) se consideran NEGRAS.
//    P4. Un nodo ROJO NO puede tener hijos ROJOS (no hay rojo-rojo).
//    P5. Todos los caminos de un nodo a sus hojas NIL tienen el mismo numero
//        de nodos NEGROS (altura negra, bh).
//  Estas propiedades garantizan altura  h <= 2*log2(n+1)  -> busqueda O(log n).
//
//  DECISIONES DE DISENO (para explicarlas al exponer)
//  --------------------------------------------------
//  * Los hijos son std::unique_ptr  -> cada nodo tiene UN solo dueno, no hay
//    fugas de memoria y no hace falta escribir destructor.
//  * El padre es un puntero CRUDO (Nodo*) -> solo "observa", no es dueno.
//    (Si el padre tambien fuera unique_ptr habria dos duenos: error.)
//  * Todo esta en el .h porque ArbolRN es una PLANTILLA (template): el
//    compilador necesita ver el codigo completo para generar ArbolRN<int,int>,
//    ArbolRN<string,int>, etc.
//  * Requisitos sobre K: debe tener operator<  y  operator<<  (para imprimir).
//
//  Compilar (g++ / MinGW):  g++ -std=c++17 -Wall -Wextra rn.cpp -o rn.exe
// ============================================================================
#pragma once

#include <memory>      // std::unique_ptr, std::make_unique
#include <utility>     // std::move
#include <functional>  // std::function (callback del recorrido inorden)
#include <cassert>     // assert (precondiciones de las rotaciones)
#include <iostream>    // std::cout (traza e imprimir)
#include <sstream>     // std::ostringstream (convertir claves a texto)
#include <string>      // std::string
#include <cstddef>     // std::size_t
#include <algorithm>   // std::max

// ----------------------------------------------------------------------------
// Color de un nodo. "enum class" evita que ROJO/NEGRO choquen con otros nombres
// y obliga a escribir Color::ROJO (mas claro y seguro que un simple enum).
// ----------------------------------------------------------------------------
enum class Color { ROJO, NEGRO };

// ----------------------------------------------------------------------------
// Nodo del arbol RN.
// ----------------------------------------------------------------------------
template<typename K, typename V>
struct NodoRN {
    K clave;                                  // clave de ordenamiento
    V valor;                                  // dato asociado a la clave
    Color color { Color::ROJO };              // todo nodo nuevo nace ROJO
    NodoRN* padre { nullptr };                // puntero crudo al padre (no es dueno)
    std::unique_ptr<NodoRN<K,V>> izq, der;    // hijos: este nodo es su unico dueno

    // Constructor: recibe por valor y MUEVE (std::move) para evitar copias
    // innecesarias cuando K o V son tipos pesados (por ejemplo std::string).
    NodoRN(K k, V v) : clave{std::move(k)}, valor{std::move(v)} {}
};

// ----------------------------------------------------------------------------
// Contadores de lo que hizo el fixup. Sirven para VERIFICAR contra el trabajo
// manual de la Actividad 1 (cuantos recoloreos y cuantas rotaciones hubo).
// ----------------------------------------------------------------------------
struct EstadisticasRN {
    int caso1   = 0;   // veces que se aplico el Caso 1 (tio ROJO -> recolorear)
    int caso2   = 0;   // veces que se aplico el Caso 2 (triangulo -> 1a rotacion)
    int caso3   = 0;   // veces que se aplico el Caso 3 (linea -> recolor + rotacion)
    int rot_izq = 0;   // total de rotaciones a la izquierda
    int rot_der = 0;   // total de rotaciones a la derecha
};

// ============================================================================
//  Clase ArbolRN
// ============================================================================
template<typename K, typename V>
class ArbolRN {
public:
    // Alias PUBLICOS (en la guia eran privados): asi el usuario puede escribir
    // ArbolRN<int,int>::Nodo* si lo necesita.
    using Nodo = NodoRN<K,V>;
    using PtrN = std::unique_ptr<Nodo>;

private:
    PtrN           raiz_;            // dueno de TODO el arbol (la raiz posee a sus hijos...)
    std::size_t    tamano_ { 0 };    // cantidad de nodos
    bool           traza_  { false };// si es true, imprime cada caso del fixup
    EstadisticasRN est_;             // contadores de casos y rotaciones

    // ------------------------------------------------------------------------
    // Utilidades pequenas
    // ------------------------------------------------------------------------
    static char letra(Color c) { return c == Color::ROJO ? 'R' : 'N'; }

    static std::string a_texto(const K& k) {      // clave -> texto (para la traza)
        std::ostringstream os;
        os << k;
        return os.str();
    }

    // ------------------------------------------------------------------------
    // propietario(n): devuelve la REFERENCIA al unique_ptr que posee a "n".
    //   - si n es la raiz  -> es raiz_
    //   - si n es hijo izq -> es padre->izq
    //   - si n es hijo der -> es padre->der
    // La necesitamos en las rotaciones: como los nodos estan en unique_ptr,
    // para "cambiar quien ocupa un lugar" hay que reasignar ese unique_ptr.
    // ------------------------------------------------------------------------
    PtrN& propietario(Nodo* n) {
        if (n->padre == nullptr) return raiz_;                    // n es la raiz
        return (n->padre->izq.get() == n) ? n->padre->izq         // n es hijo izquierdo
                                          : n->padre->der;        // n es hijo derecho
    }

    // ------------------------------------------------------------------------
    // TODO 1 - rotar_derecha(x)
    //   Precondicion: x->izq != nullptr.   x->izq (y) sube, x baja a su der.
    //
    /*        x                y
             / \              / \
            y   C    ==>     A   x
           / \                  / \
          A   B                B   C        */
    //
    //   Solo se mueven punteros: el orden BST se conserva (A < y < B < x < C).
    // ------------------------------------------------------------------------
    void rotar_derecha(Nodo* x) {
        assert(x != nullptr && x->izq != nullptr);  // precondicion de la guia

        PtrN& lugar = propietario(x);        // 1) quien posee a x hoy (raiz_ o un hijo del padre)
        PtrN  px    = std::move(lugar);      // 2) sacamos a x de su lugar: ahora "px" es su dueno
        PtrN  py    = std::move(x->izq);     // 3) sacamos a y de x: ahora "py" es su dueno
        Nodo* y     = py.get();              //    puntero crudo para trabajar con y comodamente

        x->izq = std::move(y->der);          // 4) el subarbol B (der de y) pasa a ser izq de x
        if (x->izq) x->izq->padre = x;       //    actualizamos el padre de B (si existe)

        y->padre = x->padre;                 // 5) y ocupara el lugar de x: hereda su padre
                                             //    (IMPORTANTE: antes de cambiar x->padre)
        x->padre = y;                        // 6) ahora x es hijo de y
        y->der   = std::move(px);            // 7) x pasa a ser hijo DERECHO de y (y es su dueno)

        lugar = std::move(py);               // 8) y se coloca en el lugar que dejo x
        ++est_.rot_der;                      // contabilizamos la rotacion
    }

    // ------------------------------------------------------------------------
    // TODO 2 - rotar_izquierda(x)   (simetrica a rotar_derecha)
    //   Precondicion: x->der != nullptr.   x->der (y) sube, x baja a su izq.
    //
    /*      x                    y
           / \                  / \
          A   y      ==>       x   C
             / \              / \
            B   C            A   B          */
    // ------------------------------------------------------------------------
    void rotar_izquierda(Nodo* x) {
        assert(x != nullptr && x->der != nullptr);

        PtrN& lugar = propietario(x);        // 1) lugar que ocupa x
        PtrN  px    = std::move(lugar);      // 2) sacamos a x de su lugar
        PtrN  py    = std::move(x->der);     // 3) sacamos a y (hijo derecho)
        Nodo* y     = py.get();

        x->der = std::move(y->izq);          // 4) subarbol B (izq de y) pasa a ser der de x
        if (x->der) x->der->padre = x;       //    actualizamos su padre

        y->padre = x->padre;                 // 5) y hereda el padre de x
        x->padre = y;                        // 6) x ahora es hijo de y
        y->izq   = std::move(px);            // 7) x pasa a ser hijo IZQUIERDO de y

        lugar = std::move(py);               // 8) y ocupa el lugar de x
        ++est_.rot_izq;
    }

    // ------------------------------------------------------------------------
    // TODO 3 - insertar_fixup(z)
    //   z acaba de insertarse y es ROJO. Solo puede romperse:
    //     P2 (z es la raiz roja)   o   P4 (z y su padre son ROJOS).
    //   Mientras el padre sea ROJO hay violacion. Como el padre es rojo, NO es
    //   la raiz (la raiz es negra), asi que el abuelo g existe.
    //   Se mira al TIO (el otro hijo del abuelo):
    //     Caso 1: tio ROJO            -> recolorear (padre, tio -> NEGRO; abuelo -> ROJO)
    //                                    y subir el problema: z = abuelo.
    //     Caso 2: tio NEGRO, z "triangulo" (z es hijo interno)
    //                                  -> rotar sobre el padre para convertirlo en Caso 3.
    //     Caso 3: tio NEGRO, z "linea" (z es hijo externo)
    //                                  -> padre NEGRO, abuelo ROJO, rotar sobre el abuelo.
    //   El tio NULO (nullptr) cuenta como NEGRO.
    //   Hay dos bloques simetricos: padre es hijo IZQUIERDO / hijo DERECHO.
    // ------------------------------------------------------------------------
    void insertar_fixup(Nodo* z) {
        while (z->padre != nullptr && z->padre->color == Color::ROJO) {
            Nodo* p = z->padre;              // padre de z (ROJO)
            Nodo* g = p->padre;              // abuelo de z
            assert(g != nullptr);            // un padre rojo nunca es la raiz

            if (p == g->izq.get()) {
                // ===== El padre es hijo IZQUIERDO del abuelo =====
                Nodo* tio = g->der.get();    // el tio es el hijo derecho del abuelo

                if (tio != nullptr && tio->color == Color::ROJO) {
                    // ---- CASO 1: tio ROJO -> solo recolorear ----
                    if (traza_) std::cout << "    Caso 1 (tio ROJO): padre " << a_texto(p->clave)
                                          << " y tio " << a_texto(tio->clave)
                                          << " -> NEGRO; abuelo " << a_texto(g->clave)
                                          << " -> ROJO; el problema sube a " << a_texto(g->clave) << "\n";
                    p->color   = Color::NEGRO;
                    tio->color = Color::NEGRO;
                    g->color   = Color::ROJO;
                    z = g;                   // seguimos revisando desde el abuelo
                    ++est_.caso1;
                } else {
                    if (z == p->der.get()) {
                        // ---- CASO 2: tio NEGRO y z es hijo DERECHO (triangulo izq-der) ----
                        if (traza_) std::cout << "    Caso 2 (triangulo, tio NEGRO): rotacion IZQUIERDA sobre el padre "
                                              << a_texto(p->clave) << "\n";
                        z = p;               // z pasa a ser el antiguo padre
                        rotar_izquierda(z);  // ahora el antiguo z queda arriba
                        p = z->padre;        // nuevo padre = el nodo que acaba de subir
                        ++est_.caso2;
                    }
                    // ---- CASO 3: tio NEGRO y z es hijo IZQUIERDO (linea izq-izq) ----
                    if (traza_) std::cout << "    Caso 3 (linea, tio NEGRO): padre " << a_texto(p->clave)
                                          << " -> NEGRO; abuelo " << a_texto(g->clave)
                                          << " -> ROJO; rotacion DERECHA sobre el abuelo " << a_texto(g->clave) << "\n";
                    p->color = Color::NEGRO;
                    g->color = Color::ROJO;
                    rotar_derecha(g);
                    ++est_.caso3;
                }
            } else {
                // ===== El padre es hijo DERECHO del abuelo (espejo del bloque anterior) =====
                Nodo* tio = g->izq.get();    // el tio es el hijo izquierdo del abuelo

                if (tio != nullptr && tio->color == Color::ROJO) {
                    // ---- CASO 1 (simetrico) ----
                    if (traza_) std::cout << "    Caso 1 (tio ROJO): padre " << a_texto(p->clave)
                                          << " y tio " << a_texto(tio->clave)
                                          << " -> NEGRO; abuelo " << a_texto(g->clave)
                                          << " -> ROJO; el problema sube a " << a_texto(g->clave) << "\n";
                    p->color   = Color::NEGRO;
                    tio->color = Color::NEGRO;
                    g->color   = Color::ROJO;
                    z = g;
                    ++est_.caso1;
                } else {
                    if (z == p->izq.get()) {
                        // ---- CASO 2 (simetrico): z es hijo IZQUIERDO (triangulo der-izq) ----
                        if (traza_) std::cout << "    Caso 2 (triangulo, tio NEGRO): rotacion DERECHA sobre el padre "
                                              << a_texto(p->clave) << "\n";
                        z = p;
                        rotar_derecha(z);
                        p = z->padre;
                        ++est_.caso2;
                    }
                    // ---- CASO 3 (simetrico): z es hijo DERECHO (linea der-der) ----
                    if (traza_) std::cout << "    Caso 3 (linea, tio NEGRO): padre " << a_texto(p->clave)
                                          << " -> NEGRO; abuelo " << a_texto(g->clave)
                                          << " -> ROJO; rotacion IZQUIERDA sobre el abuelo " << a_texto(g->clave) << "\n";
                    p->color = Color::NEGRO;
                    g->color = Color::ROJO;
                    rotar_izquierda(g);
                    ++est_.caso3;
                }
            }
        }
        // Propiedad P2: la raiz siempre termina NEGRA (el Caso 1 puede dejarla roja).
        if (raiz_->color == Color::ROJO) {
            if (traza_) std::cout << "    Raiz " << a_texto(raiz_->clave) << " estaba ROJA -> NEGRA (P2)\n";
            raiz_->color = Color::NEGRO;
        }
    }

    // ------------------------------------------------------------------------
    // Auxiliar de altura negra: cuenta los nodos NEGROS desde n hasta una hoja
    // NIL, CONTANDO a n (si es negro) y a la hoja NIL (que vale 1).
    // Devuelve -1 si dos caminos tienen distinta cantidad de negros (P5 rota).
    // ------------------------------------------------------------------------
    int negros_desde(const Nodo* n) const {
        if (n == nullptr) return 1;                         // hoja NIL = 1 nodo negro
        int bi = negros_desde(n->izq.get());                // negros por la izquierda
        int bd = negros_desde(n->der.get());                // negros por la derecha
        if (bi < 0 || bd < 0 || bi != bd) return -1;        // inconsistencia
        return bi + (n->color == Color::NEGRO ? 1 : 0);     // sumamos a n si es negro
    }

    // ------------------------------------------------------------------------
    // TODO 4 - altura_negra_impl(n)
    //   bh(n) = cantidad de nodos negros en el camino desde n hasta una hoja
    //           NIL, SIN contar a n pero CONTANDO la hoja NIL (convencion de
    //           CLRS).  Con esta convencion:
    //              10,20,30            -> bh(raiz) = 1   (caso de prueba P2)
    //              10,20,30,15,25      -> bh(raiz) = 2   (Secuencia A)
    //   Retorna -1 si el arbol es inconsistente.
    // ------------------------------------------------------------------------
    int altura_negra_impl(Nodo* n) const {
        if (n == nullptr) return 0;                         // arbol vacio
        int total = negros_desde(n);                        // negros contando a n y a NIL
        if (total < 0) return -1;                           // inconsistencia propagada
        return total - (n->color == Color::NEGRO ? 1 : 0);  // restamos a n (no se cuenta)
    }

    // ------------------------------------------------------------------------
    // Validador COMPLETO (todas las propiedades + orden BST + punteros padre).
    // Devuelve la cantidad de negros o -1 y deja el motivo en "motivo".
    // minimo/maximo son las cotas BST que debe respetar este subarbol.
    // ------------------------------------------------------------------------
    int validar_rec(const Nodo* n, const K* minimo, const K* maximo, std::string& motivo) const {
        if (n == nullptr) return 1;                         // NIL es negro (cuenta 1)

        if (minimo && !(*minimo < n->clave)) { motivo = "orden BST roto en " + a_texto(n->clave); return -1; }
        if (maximo && !(n->clave < *maximo)) { motivo = "orden BST roto en " + a_texto(n->clave); return -1; }

        if (n->izq && n->izq->padre != n) { motivo = "puntero padre incorrecto (hijo izq de " + a_texto(n->clave) + ")"; return -1; }
        if (n->der && n->der->padre != n) { motivo = "puntero padre incorrecto (hijo der de " + a_texto(n->clave) + ")"; return -1; }

        if (n->color == Color::ROJO) {                      // P4: rojo no puede tener hijo rojo
            if ((n->izq && n->izq->color == Color::ROJO) ||
                (n->der && n->der->color == Color::ROJO)) {
                motivo = "rojo-rojo en " + a_texto(n->clave);
                return -1;
            }
        }

        int bi = validar_rec(n->izq.get(), minimo, &n->clave, motivo);   // izquierda: todo < n
        if (bi < 0) return -1;
        int bd = validar_rec(n->der.get(), &n->clave, maximo, motivo);   // derecha: todo > n
        if (bd < 0) return -1;

        if (bi != bd) {                                     // P5
            motivo = "alturas negras distintas en " + a_texto(n->clave);
            return -1;
        }
        return bi + (n->color == Color::NEGRO ? 1 : 0);
    }

    // Altura en NODOS (arbol vacio = 0, un solo nodo = 1).
    int altura_rec(const Nodo* n) const {
        if (n == nullptr) return 0;
        return 1 + std::max(altura_rec(n->izq.get()), altura_rec(n->der.get()));
    }

    // Recorrido inorden (izq, nodo, der) -> visita las claves en orden ascendente.
    void inorden_rec(const Nodo* n, const std::function<void(const K&, const V&)>& f) const {
        if (n == nullptr) return;
        inorden_rec(n->izq.get(), f);
        f(n->clave, n->valor);
        inorden_rec(n->der.get(), f);
    }

    // Notacion en una linea:  clave(color)[hijo_izq,hijo_der]   ('-' = NIL)
    void en_linea_rec(const Nodo* n, std::ostringstream& os) const {
        if (n == nullptr) { os << '-'; return; }
        os << n->clave << '(' << letra(n->color) << ')';
        if (n->izq || n->der) {                             // las hojas no llevan corchetes
            os << '[';
            en_linea_rec(n->izq.get(), os);
            os << ',';
            en_linea_rec(n->der.get(), os);
            os << ']';
        }
    }

    // Dibujo horizontal: la derecha va ARRIBA y la izquierda ABAJO (giro de 90 grados).
    void imprimir_rec(const Nodo* n, int nivel) const {
        if (n == nullptr) return;
        imprimir_rec(n->der.get(), nivel + 1);
        std::cout << std::string(static_cast<std::size_t>(nivel) * 6, ' ')
                  << n->clave << '(' << letra(n->color) << ")\n";
        imprimir_rec(n->izq.get(), nivel + 1);
    }

public:
    // ------------------------------------------------------------------------
    // insertar: 1) insercion BST normal  2) nodo nuevo ROJO  3) fixup.
    //   Si la clave ya existe NO se duplica: se actualiza su valor.
    // ------------------------------------------------------------------------
    void insertar(K clave, V valor) {
        // --- 1) Buscar el padre donde quedara el nodo nuevo (descenso BST) ---
        Nodo* padre  = nullptr;
        Nodo* actual = raiz_.get();
        while (actual != nullptr) {
            padre = actual;
            if (clave < actual->clave)       actual = actual->izq.get();   // ir a la izquierda
            else if (actual->clave < clave)  actual = actual->der.get();   // ir a la derecha
            else {                                                         // clave repetida
                actual->valor = std::move(valor);                          // solo actualizamos el valor
                return;
            }
        }

        // --- 2) Crear el nodo (nace ROJO) y enlazarlo ---
        PtrN nuevo = std::make_unique<Nodo>(std::move(clave), std::move(valor));
        Nodo* z = nuevo.get();               // puntero crudo que usaremos en el fixup
        z->padre = padre;                    // enlazamos hacia arriba

        if (padre == nullptr)                // arbol vacio: z es la raiz
            raiz_ = std::move(nuevo);
        else if (z->clave < padre->clave)    // OJO: usamos z->clave (clave ya fue movida)
            padre->izq = std::move(nuevo);
        else
            padre->der = std::move(nuevo);
        ++tamano_;

        if (traza_) std::cout << "  Insertar " << a_texto(z->clave) << " (ROJO)"
                              << (padre ? "" : "  -> es la raiz") << "\n";

        // --- 3) Restaurar las propiedades RN ---
        insertar_fixup(z);
    }

    // Busqueda BST estandar (iterativa). Devuelve nullptr si no existe. O(log n).
    Nodo* buscar(const K& k) const {
        Nodo* actual = raiz_.get();
        while (actual != nullptr) {
            if (k < actual->clave)            actual = actual->izq.get();
            else if (actual->clave < k)       actual = actual->der.get();
            else                              return actual;     // encontrada
        }
        return nullptr;
    }

    Nodo* raiz() const { return raiz_.get(); }
    int   altura_negra(Nodo* n) const { return altura_negra_impl(n); }

    // ---- Extras utiles para pruebas y para la exposicion ----
    int         altura()  const { return altura_rec(raiz_.get()); }  // en nodos
    std::size_t tamano()  const { return tamano_; }
    bool        vacio()   const { return raiz_ == nullptr; }

    void inorden(const std::function<void(const K&, const V&)>& f) const { inorden_rec(raiz_.get(), f); }

    // Verifica TODAS las propiedades. Si falla y se pasa "motivo", dice por que.
    bool es_valido(std::string* motivo = nullptr) const {
        std::string m;
        bool ok = true;
        if (raiz_ != nullptr) {
            if (raiz_->color != Color::NEGRO) { m = "la raiz no es NEGRA (P2)"; ok = false; }
            else if (raiz_->padre != nullptr) { m = "la raiz tiene padre";        ok = false; }
            else if (validar_rec(raiz_.get(), nullptr, nullptr, m) < 0) ok = false;
        }
        if (!ok && motivo) *motivo = m;
        return ok;
    }

    std::string en_linea() const {
        std::ostringstream os;
        en_linea_rec(raiz_.get(), os);
        return os.str();
    }

    void imprimir() const {
        if (!raiz_) { std::cout << "  (arbol vacio)\n"; return; }
        imprimir_rec(raiz_.get(), 0);
    }

    void activar_traza(bool v)                 { traza_ = v; }
    const EstadisticasRN& estadisticas() const { return est_; }
    void reiniciar_estadisticas()              { est_ = EstadisticasRN{}; }
};
