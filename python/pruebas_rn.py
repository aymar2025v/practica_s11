"""
pruebas_rn.py  -  Pruebas de rn.py, bst.py y avl.py (equivalente a rn.cpp)
SIS210 - Practica Semana 11

Ejecutar:   python pruebas_rn.py
Sirve para asegurarse de que las estructuras son correctas ANTES de correr el
experimento (Actividad 3) y el SIGA-RN (Actividad 4).
"""
import random
import sys

from rn import ArbolRN, NEGRO
from bst import BST
from avl import AVL

total = 0      # verificaciones realizadas
fallos = 0     # verificaciones fallidas


def verificar(condicion, descripcion):
    """Imprime [OK] o [FALLO] y lleva la cuenta (como la macro VERIFICAR de C++)."""
    global total, fallos
    total += 1
    if condicion:
        print(f"  [OK]    {descripcion}")
    else:
        fallos += 1
        print(f"  [FALLO] {descripcion}")


def construir(claves):
    """Crea un ArbolRN e inserta las claves en orden."""
    a = ArbolRN()
    for k in claves:
        a.insertar(k, k)
    return a


def main():
    print("=== PRUEBAS ARBOL ROJO-NEGRO (Python) ===")

    # ---- P1: raiz sola ----
    print("\n==== P1: insercion simple ====")
    a = construir([10])
    verificar(a.raiz.color == NEGRO, "la raiz 10 es NEGRA")
    verificar(a.en_linea() == "10(N)", "forma: 10(N)")

    # ---- P2: 10, 20, 30 -> Caso 3 ----
    print("\n==== P2: 10, 20, 30 (Caso 3, rotacion simple izquierda) ====")
    a = construir([10, 20, 30])
    verificar(a.raiz.clave == 20, "la raiz es 20")
    verificar(a.altura_negra() == 1, "bh(raiz) = 1")
    verificar(a.en_linea() == "20(N)[10(R),30(R)]", "forma: 20(N)[10(R),30(R)]")
    verificar(a.estadisticas["caso3"] == 1 and a.estadisticas["caso1"] == 0, "1 Caso 3 y 0 Caso 1")

    # ---- P3: 30, 10, 20 -> Caso 2 + 3 ----
    print("\n==== P3: 30, 10, 20 (Caso 2 + 3, doble rotacion) ====")
    a = construir([30, 10, 20])
    verificar(a.raiz.clave == 20 and a.raiz.color == NEGRO, "20 sube a la raiz y es NEGRA")
    verificar(a.estadisticas["caso2"] == 1 and a.estadisticas["caso3"] == 1, "Caso 2 y Caso 3")
    verificar(a.estadisticas["rot_izq"] == 1 and a.estadisticas["rot_der"] == 1, "1 rot. izq + 1 rot. der")

    # ---- P4: 1..15 ----
    print("\n==== P4: 15 inserciones (1..15) ====")
    a = construir(range(1, 16))
    ok, motivo = a.es_valido()
    verificar(a.altura_negra() >= 0, "altura negra consistente")
    verificar(ok, "todas las propiedades RN se cumplen " + motivo)
    verificar(len(a) == 15, "tamano = 15")

    # ---- Secuencia A ----
    print("\n==== Secuencia A: 10, 20, 30, 15, 25 ====")
    a = construir([10, 20, 30, 15, 25])
    verificar(a.en_linea() == "20(N)[10(N)[-,15(R)],30(N)[25(R),-]]", "forma exacta esperada")
    verificar(a.altura_negra() == 2, "bh = 2")
    verificar(a.buscar(15) is not None and a.buscar(99) is None, "buscar(15) existe, buscar(99) no")

    # ---- Secuencia B (y ampliada) ----
    print("\n==== Secuencia B: 7, 3, 18, 10, 22, 8, 11, 26 ====")
    b = construir([7, 3, 18, 10, 22, 8, 11, 26])
    verificar(b.en_linea() == "7(N)[3(N),18(R)[10(N)[8(R),11(R)],22(N)[-,26(R)]]]", "forma exacta esperada")
    verificar(b.estadisticas["caso1"] == 2 and b.estadisticas["caso2"] == 0
              and b.estadisticas["caso3"] == 0, "solo ocurre el Caso 1 (2 veces)")
    verificar(b.estadisticas["rot_izq"] == 0 and b.estadisticas["rot_der"] == 0, "no hay rotaciones")
    b.estadisticas = {k: 0 for k in b.estadisticas}          # reiniciar contadores
    b.insertar(15, 15)
    verificar(b.en_linea() == "10(N)[7(R)[3(N),8(N)],18(R)[11(N)[-,15(R)],22(N)[-,26(R)]]]",
              "con +15 la raiz pasa a ser 10")
    verificar(b.estadisticas["caso1"] == 1 and b.estadisticas["caso2"] == 1
              and b.estadisticas["caso3"] == 1, "insertar 15 activa los Casos 1, 2 y 3")

    # ---- Estres: validar tras cada insercion ----
    print("\n==== Estres: validar tras CADA insercion (n = 1000) ====")
    n = 1000
    random.seed(2026)
    aleatorio = random.sample(range(1, n * 10), n)
    for nombre, claves in (("creciente", list(range(1, n + 1))),
                           ("decreciente", list(range(n, 0, -1))),
                           ("aleatorio", aleatorio)):
        a = ArbolRN()
        todo_ok = True
        for k in claves:
            a.insertar(k, k)
            if not a.es_valido()[0]:
                todo_ok = False
                break
        verificar(todo_ok, f"RN valido tras cada insercion ({nombre}); altura = {a.altura()}")

    # ---- BST y AVL ----
    print("\n==== BST y AVL ====")
    bst = BST()
    for k in range(1, 501):
        bst.insertar(k)
    verificar(bst.altura() == 500, "BST con claves crecientes se degenera: altura = n = 500")
    avl = AVL()
    for k in range(1, 1001):
        avl.insertar(k, k)
        if not avl.es_valido():
            break
    verificar(avl.es_valido() and avl.altura() <= 14, f"AVL valido con claves crecientes (altura = {avl.altura()})")

    # ---- inorden y duplicados ----
    print("\n==== Inorden y duplicados ====")
    a = construir([50, 20, 80, 10, 30, 70, 90, 25])
    salida = []
    a.inorden(lambda k, v: salida.append(k))
    verificar(salida == sorted(salida) and len(salida) == 8, "inorden entrega las claves ordenadas")
    a.insertar(50, "nuevo")
    verificar(len(a) == 8 and a.buscar(50).valor == "nuevo", "clave repetida actualiza el valor sin duplicar")

    print("\n=============================================")
    print(f"RESUMEN: {total - fallos} de {total} verificaciones correctas", end="")
    print("  -> TODO OK" if fallos == 0 else f"  -> HAY {fallos} FALLO(S)")
    return 0 if fallos == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
