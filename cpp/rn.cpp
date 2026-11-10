// ============================================================================
//  rn.cpp  -  Pruebas del arbol Rojo-Negro (Actividades 1 y 2)
//  SIS210 - Practica Semana 11
//
//  Compilar y ejecutar (g++ / MinGW en la terminal de VS Code):
//      g++ -std=c++17 -Wall -Wextra -O2 rn.cpp -o rn.exe
//      .\rn.exe
//
//  Este archivo NO implementa el arbol (eso esta en rn.h); solo lo PRUEBA.
//  En vez de assert() usamos una macro VERIFICAR que:
//    - imprime [OK] o [FALLO] por cada comprobacion (se ve en la exposicion),
//    - NO aborta el programa en el primer error,
//    - sigue funcionando aunque se compile con -DNDEBUG (assert se desactivaria).
// ============================================================================
#include "rn.h"

#include <iostream>   // std::cout
#include <vector>     // std::vector
#include <string>     // std::string
#include <random>     // std::mt19937 (numeros pseudoaleatorios reproducibles)
#include <algorithm>  // std::shuffle
#include <numeric>    // std::iota
#include <cmath>      // std::log2

// ---------------------------------------------------------------------------
// Contadores globales de verificaciones.
// ---------------------------------------------------------------------------
static int g_total  = 0;   // cuantas verificaciones se hicieron
static int g_fallos = 0;   // cuantas fallaron

// Macro de verificacion: evalua "cond"; imprime OK o FALLO con la linea.
// do { ... } while (0) permite usarla como una sentencia normal terminada en ';'.
#define VERIFICAR(cond, descripcion)                                              \
    do {                                                                          \
        ++g_total;                                                                \
        if (cond) {                                                               \
            std::cout << "  [OK]    " << descripcion << "\n";                     \
        } else {                                                                  \
            ++g_fallos;                                                           \
            std::cout << "  [FALLO] " << descripcion                              \
                      << "   (linea " << __LINE__ << ")\n";                       \
        }                                                                         \
    } while (0)

// Alias para escribir menos: arbol de claves int con valores int (como la guia).
using Arbol = ArbolRN<int, int>;

// Imprime un titulo de seccion.
static void titulo(const std::string& t) {
    std::cout << "\n==== " << t << " ====\n";
}

// ---------------------------------------------------------------------------
// P1: insercion simple -> solo se inserta la raiz; debe quedar NEGRA.
// ---------------------------------------------------------------------------
static void prueba_P1() {
    titulo("P1: insercion simple (raiz)");
    Arbol arbol;
    arbol.insertar(10, 10);                                   // unica clave

    VERIFICAR(arbol.raiz()->color == Color::NEGRO, "la raiz 10 es NEGRA (P2)");
    VERIFICAR(arbol.tamano() == 1,                  "tamano = 1");
    VERIFICAR(arbol.es_valido(),                    "el arbol cumple todas las propiedades RN");
    VERIFICAR(arbol.en_linea() == "10(N)",          "forma: 10(N)");
}

// ---------------------------------------------------------------------------
// P2: claves 10, 20, 30.
//   - 20 es hijo derecho de 10 (rojo), 30 es hijo derecho de 20 (rojo).
//   - El tio de 30 es NULO (= negro)  ->  es el CASO 3 (linea der-der):
//     una rotacion IZQUIERDA sobre 10 + recoloreo.
//   NOTA: la guia rotula este caso como "Caso 1 (tio ROJO)"; en realidad el
//   Caso 1 NO ocurre aqui (no hay tio rojo). Las aserciones de la guia
//   (raiz = 20, bh = 1) si son correctas y se verifican abajo.
// ---------------------------------------------------------------------------
static void prueba_P2() {
    titulo("P2: claves 10, 20, 30 (Caso 3: rotacion simple izquierda)");
    Arbol a2;
    for (int k : {10, 20, 30}) a2.insertar(k, k);

    VERIFICAR(a2.raiz()->clave == 20,                "la raiz debe ser 20");
    VERIFICAR(a2.altura_negra(a2.raiz()) == 1,       "bh(raiz) = 1");
    VERIFICAR(a2.raiz()->color == Color::NEGRO,      "la raiz 20 es NEGRA");
    VERIFICAR(a2.raiz()->izq->color == Color::ROJO,  "el hijo izquierdo 10 es ROJO");
    VERIFICAR(a2.raiz()->der->color == Color::ROJO,  "el hijo derecho 30 es ROJO");
    VERIFICAR(a2.en_linea() == "20(N)[10(R),30(R)]", "forma: 20(N)[10(R),30(R)]");
    VERIFICAR(a2.estadisticas().caso3 == 1 &&
              a2.estadisticas().caso1 == 0 &&
              a2.estadisticas().caso2 == 0,          "se aplico UNA vez el Caso 3 (y ningun Caso 1 ni 2)");
    VERIFICAR(a2.estadisticas().rot_izq == 1 &&
              a2.estadisticas().rot_der == 0,        "exactamente 1 rotacion izquierda");
    VERIFICAR(a2.es_valido(),                        "el arbol cumple todas las propiedades RN");
}

// ---------------------------------------------------------------------------
// P3: claves 30, 10, 20 -> caso "triangulo" (izq-der): Caso 2 + Caso 3.
//   Doble rotacion: izquierda sobre 10, luego derecha sobre 30. Sube el 20.
// ---------------------------------------------------------------------------
static void prueba_P3() {
    titulo("P3: claves 30, 10, 20 (Caso 2 + Caso 3: doble rotacion LR)");
    Arbol a3;
    for (int k : {30, 10, 20}) a3.insertar(k, k);

    VERIFICAR(a3.raiz()->clave == 20,                "20 sube a la raiz");
    VERIFICAR(a3.raiz()->color == Color::NEGRO,      "la raiz 20 es NEGRA");
    VERIFICAR(a3.en_linea() == "20(N)[10(R),30(R)]", "forma: 20(N)[10(R),30(R)]");
    VERIFICAR(a3.estadisticas().caso2 == 1 &&
              a3.estadisticas().caso3 == 1,          "se aplicaron Caso 2 y Caso 3 (una vez cada uno)");
    VERIFICAR(a3.estadisticas().rot_izq == 1 &&
              a3.estadisticas().rot_der == 1,        "1 rotacion izquierda y 1 derecha (doble rotacion)");
    VERIFICAR(a3.es_valido(),                        "el arbol cumple todas las propiedades RN");
}

// ---------------------------------------------------------------------------
// P4: 15 inserciones consecutivas (1..15) -> consistencia de altura negra.
// ---------------------------------------------------------------------------
static void prueba_P4() {
    titulo("P4: 15 inserciones (1..15) y consistencia de altura negra");
    Arbol a4;
    for (int k = 1; k <= 15; ++k) a4.insertar(k, k);

    std::cout << "  Arbol resultante: " << a4.en_linea() << "\n";
    VERIFICAR(a4.altura_negra(a4.raiz()) >= 0,    "altura negra consistente (-1 seria inconsistente)");
    VERIFICAR(a4.raiz()->color == Color::NEGRO,   "la raiz es NEGRA");
    VERIFICAR(a4.tamano() == 15,                  "tamano = 15");
    VERIFICAR(a4.es_valido(),                     "TODAS las propiedades RN y el orden BST se cumplen");
    // Cota teorica (altura en aristas <= 2*log2(n+1)); nuestra altura esta en nodos.
    VERIFICAR(a4.altura() - 1 <= 2 * std::log2(15 + 1), "altura <= 2*log2(n+1)");
    std::cout << "  altura (nodos) = " << a4.altura()
              << ", bh(raiz) = " << a4.altura_negra(a4.raiz()) << "\n";
}

// ---------------------------------------------------------------------------
// Secuencia A de la guia (Actividad 1.1): 10, 20, 30, 15, 25
//   Resultado correcto:   20(N) con hijos 10(N) y 30(N);
//                         15(R) hijo derecho de 10;  25(R) hijo izquierdo de 30.
//   (El "criterio de correccion" de la guia dice 25(N), pero 25 es ROJO.)
// ---------------------------------------------------------------------------
static void prueba_secuencia_A() {
    titulo("Secuencia A: 10, 20, 30, 15, 25");
    Arbol arbol;
    for (int k : {10, 20, 30, 15, 25}) arbol.insertar(k, k);

    std::cout << "  Arbol resultante: " << arbol.en_linea() << "\n";
    VERIFICAR(arbol.raiz()->color == Color::NEGRO,                 "raiz NEGRA (P2)");
    VERIFICAR(arbol.altura_negra(arbol.raiz()) == 2,               "bh = 2");
    VERIFICAR(arbol.buscar(15) != nullptr,                         "buscar(15) lo encuentra");
    VERIFICAR(arbol.buscar(99) == nullptr,                         "buscar(99) devuelve nullptr");
    VERIFICAR(arbol.en_linea() == "20(N)[10(N)[-,15(R)],30(N)[25(R),-]]",
                                                                   "forma exacta esperada");
    VERIFICAR(arbol.estadisticas().caso1 == 1 &&
              arbol.estadisticas().caso3 == 1,                     "1 Caso 1 (al insertar 15) y 1 Caso 3 (al insertar 30)");
    VERIFICAR(arbol.es_valido(),                                   "el arbol cumple todas las propiedades RN");
    std::cout << "Secuencia A: OK\n";
}

// ---------------------------------------------------------------------------
// Secuencia B de la guia (Actividad 1.2): 7, 3, 18, 10, 22, 8, 11, 26
//   Hallazgo: con estas 8 claves SOLO ocurre el Caso 1 (dos veces) y NO hay
//   rotaciones. Al agregar la clave 15 se activan los tres casos en cadena.
// ---------------------------------------------------------------------------
static void prueba_secuencia_B() {
    titulo("Secuencia B: 7, 3, 18, 10, 22, 8, 11, 26");
    Arbol b;
    for (int k : {7, 3, 18, 10, 22, 8, 11, 26}) b.insertar(k, k);

    std::cout << "  Arbol resultante: " << b.en_linea() << "\n";
    VERIFICAR(b.en_linea() == "7(N)[3(N),18(R)[10(N)[8(R),11(R)],22(N)[-,26(R)]]]",
                                                  "forma exacta esperada");
    VERIFICAR(b.estadisticas().caso1 == 2,        "Caso 1 ocurre 2 veces (claves 10 y 8)");
    VERIFICAR(b.estadisticas().caso2 == 0 &&
              b.estadisticas().caso3 == 0,        "NO hay Caso 2 ni Caso 3");
    VERIFICAR(b.estadisticas().rot_izq == 0 &&
              b.estadisticas().rot_der == 0,      "NO hay rotaciones");
    VERIFICAR(b.es_valido(),                      "el arbol cumple todas las propiedades RN");

    // Ampliacion sugerida: insertar 15 activa Caso 1 -> Caso 2 -> Caso 3.
    titulo("Secuencia B ampliada: ... + 15 (activa los tres casos)");
    b.reiniciar_estadisticas();                   // contamos solo lo que hace el 15
    b.insertar(15, 15);
    std::cout << "  Arbol resultante: " << b.en_linea() << "\n";
    VERIFICAR(b.en_linea() == "10(N)[7(R)[3(N),8(N)],18(R)[11(N)[-,15(R)],22(N)[-,26(R)]]]",
                                                  "forma exacta esperada (raiz = 10)");
    VERIFICAR(b.estadisticas().caso1 == 1 &&
              b.estadisticas().caso2 == 1 &&
              b.estadisticas().caso3 == 1,        "la insercion de 15 aplica Caso 1, Caso 2 y Caso 3");
    VERIFICAR(b.estadisticas().rot_izq == 1 &&
              b.estadisticas().rot_der == 1,      "doble rotacion (derecha en 18, izquierda en 7)");
    VERIFICAR(b.es_valido(),                      "el arbol cumple todas las propiedades RN");
}

// ---------------------------------------------------------------------------
// Prueba de ESTRES: se valida el arbol COMPLETO despues de CADA insercion
// en tres escenarios (creciente, decreciente, aleatorio).
// ---------------------------------------------------------------------------
static void prueba_estres() {
    titulo("Estres: validar tras cada insercion (n = 2000)");
    const int n = 2000;

    std::vector<int> crec(n);
    std::iota(crec.begin(), crec.end(), 1);                          // 1,2,...,n
    std::vector<int> decr(crec.rbegin(), crec.rend());               // n,...,2,1
    std::vector<int> alea = crec;
    std::mt19937 rng(2026);                                          // semilla fija = reproducible
    std::shuffle(alea.begin(), alea.end(), rng);                     // permutacion aleatoria

    struct Escenario { const char* nombre; const std::vector<int>* claves; };
    Escenario escenarios[] = { {"creciente", &crec}, {"decreciente", &decr}, {"aleatorio", &alea} };

    for (const auto& e : escenarios) {
        Arbol arbol;
        bool todo_ok = true;
        std::string motivo;
        for (int k : *e.claves) {
            arbol.insertar(k, k);
            if (!arbol.es_valido(&motivo)) { todo_ok = false; break; }   // paramos al primer error
        }
        std::cout << "  [" << e.nombre << "] altura = " << arbol.altura()
                  << " nodos, bh = " << arbol.altura_negra(arbol.raiz())
                  << ", casos 1/2/3 = " << arbol.estadisticas().caso1 << "/"
                  << arbol.estadisticas().caso2 << "/" << arbol.estadisticas().caso3 << "\n";
        VERIFICAR(todo_ok, std::string("valido tras cada insercion (") + e.nombre + ")" +
                           (todo_ok ? "" : " -> " + motivo));
        VERIFICAR(arbol.tamano() == static_cast<std::size_t>(n), std::string("tamano = n (") + e.nombre + ")");
        VERIFICAR(arbol.altura() - 1 <= 2 * std::log2(n + 1.0),  std::string("altura <= 2*log2(n+1) (") + e.nombre + ")");
    }
}

// ---------------------------------------------------------------------------
// Otras pruebas: claves repetidas, inorden y arbol generico con string.
// ---------------------------------------------------------------------------
static void prueba_extras() {
    titulo("Extras: duplicados, inorden y plantilla con std::string");

    // --- clave repetida: no se duplica, se actualiza el valor ---
    Arbol d;
    d.insertar(10, 111);
    d.insertar(10, 222);
    VERIFICAR(d.tamano() == 1,                  "insertar una clave repetida no agrega nodos");
    VERIFICAR(d.buscar(10)->valor == 222,       "el valor de la clave repetida se actualiza");

    // --- inorden devuelve las claves ORDENADAS ---
    Arbol e;
    for (int k : {50, 20, 80, 10, 30, 70, 90, 25}) e.insertar(k, k * 2);
    std::vector<int> orden;
    e.inorden([&](const int& clave, const int&) { orden.push_back(clave); });
    VERIFICAR(std::is_sorted(orden.begin(), orden.end()), "inorden entrega las claves en orden ascendente");
    VERIFICAR(orden.size() == 8,                          "inorden visita las 8 claves");

    // --- la misma clase sirve para otros tipos (plantilla) ---
    ArbolRN<std::string, int> frutas;
    for (std::string f : {"mango", "pera", "manzana", "uva", "kiwi"})
        frutas.insertar(f, static_cast<int>(f.size()));
    std::vector<std::string> ordenadas;
    frutas.inorden([&](const std::string& k, const int&) { ordenadas.push_back(k); });
    VERIFICAR(std::is_sorted(ordenadas.begin(), ordenadas.end()), "ArbolRN<string,int> ordena alfabeticamente");
    VERIFICAR(frutas.buscar("kiwi") != nullptr && frutas.buscar("kiwi")->valor == 4,
                                                          "buscar(\"kiwi\") devuelve su valor (4)");
    VERIFICAR(frutas.es_valido(),                         "arbol de strings valido");

    // --- arbol vacio ---
    Arbol v;
    VERIFICAR(v.vacio() && v.es_valido() && v.buscar(1) == nullptr, "arbol vacio: valido y sin resultados");
}

// ---------------------------------------------------------------------------
// main: ejecuta todo en orden y devuelve 0 si TODO paso (util para scripts).
// ---------------------------------------------------------------------------
int main() {
    std::cout << "=== PRUEBAS ARBOL ROJO-NEGRO (C++17) ===\n";

    prueba_P1();
    prueba_P2();
    prueba_P3();
    prueba_P4();
    prueba_secuencia_A();
    prueba_secuencia_B();
    prueba_estres();
    prueba_extras();

    std::cout << "\n=============================================\n";
    std::cout << "RESUMEN: " << (g_total - g_fallos) << " de " << g_total
              << " verificaciones correctas";
    if (g_fallos == 0) std::cout << "  -> TODO OK\n";
    else               std::cout << "  -> HAY " << g_fallos << " FALLO(S)\n";

    return (g_fallos == 0) ? 0 : 1;       // codigo de salida: 0 = exito
}
