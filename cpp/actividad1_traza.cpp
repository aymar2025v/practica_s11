// ============================================================================
//  actividad1_traza.cpp  -  Inserciones PASO A PASO (apoyo a la Actividad 1)
//  SIS210 - Practica Semana 11
//
//  Para cada clave de las secuencias A y B imprime:
//     - que caso(s) de fixup se aplicaron (Caso 1, 2, 3) y que rotacion,
//     - el arbol resultante dibujado (derecha arriba, izquierda abajo),
//     - el arbol en notacion lineal  clave(color)[izq,der],
//     - la altura negra.
//  Sirve para COMPARAR con tus dibujos hechos a mano en la guia.
//
//  Compilar y ejecutar:
//      g++ -std=c++17 -Wall -Wextra -O2 actividad1_traza.cpp -o actividad1_traza.exe
//      .\actividad1_traza.exe
// ============================================================================
#include "rn.h"

#include <iostream>
#include <vector>
#include <string>

using Arbol = ArbolRN<int, int>;

// ---------------------------------------------------------------------------
// Inserta una a una las claves de "secuencia" mostrando todo el detalle.
// ---------------------------------------------------------------------------
static void tracear(const std::string& nombre, const std::vector<int>& secuencia) {
    std::cout << "\n############################################################\n";
    std::cout << "# " << nombre << "\n";
    std::cout << "############################################################\n";

    Arbol arbol;
    arbol.activar_traza(true);              // que el arbol cuente en voz alta lo que hace

    int paso = 0;
    for (int k : secuencia) {
        std::cout << "\n--- Paso " << ++paso << " ---\n";

        arbol.reiniciar_estadisticas();     // contamos solo lo de ESTE paso
        arbol.insertar(k, k);               // (la traza imprime los casos aplicados)

        const EstadisticasRN& e = arbol.estadisticas();
        if (e.caso1 + e.caso2 + e.caso3 == 0)
            std::cout << "    (no hay violacion: no se requiere fixup)\n";

        std::cout << "  Arbol tras el paso " << paso << ":\n";
        arbol.imprimir();
        std::cout << "  Notacion lineal: " << arbol.en_linea() << "\n";
        std::cout << "  Raiz: " << arbol.raiz()->clave
                  << "(" << (arbol.raiz()->color == Color::NEGRO ? "N" : "R") << ")"
                  << "   bh(raiz) = " << arbol.altura_negra(arbol.raiz())
                  << "   altura = " << arbol.altura() << " nodos\n";

        // Autocomprobacion: despues de cada paso el arbol DEBE ser valido.
        std::string motivo;
        if (!arbol.es_valido(&motivo))
            std::cout << "  *** ERROR: el arbol es invalido: " << motivo << " ***\n";
    }
}

int main() {
    tracear("Secuencia A: 10, 20, 30, 15, 25",                  {10, 20, 30, 15, 25});
    tracear("Secuencia B: 7, 3, 18, 10, 22, 8, 11, 26",         {7, 3, 18, 10, 22, 8, 11, 26});
    tracear("Secuencia B ampliada (+15): provoca Casos 1, 2 y 3", {7, 3, 18, 10, 22, 8, 11, 26, 15});
    return 0;
}
