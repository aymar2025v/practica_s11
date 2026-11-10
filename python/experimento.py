"""
experimento.py  -  Actividad 3: BST vs AVL vs Rojo-Negro
SIS210 - Practica Semana 11

Compara la ALTURA de los tres arboles al insertar n claves en tres escenarios:
    - aleatorio   (claves distintas en orden al azar)
    - creciente   (1, 2, 3, ..., n)
    - decreciente (n, ..., 3, 2, 1)

Ejecutar (terminal de VS Code, Windows):
    python experimento.py            -> n = 100, 500, 1000, 5000, 10000 (completo)
    python experimento.py --rapido   -> n = 100, 500, 1000 (para probar rapido)

Genera ademas:  resultados_experimento.csv   (para pegar en Excel / el informe)

Convenciones:
    * Altura en NODOS (vacio = 0, un nodo = 1) en los tres arboles.
    * Cotas teoricas (las mismas de la guia):
          AVL : h <= 1.44 * log2(n + 2)
          RN  : h <= 2    * log2(n + 1)      (esta cota es para altura en ARISTAS;
                                              por eso para RN se compara h - 1)
    * CORRECCION: la tabla de la guia traia cotas de AVL (23, 30, 37) que NO
      coinciden con la formula del propio script. Los valores correctos son los
      que imprime este programa (p. ej. n = 100 -> 9.6, n = 1000 -> 14.4,
      n = 10000 -> 19.1).
"""
import argparse
import csv
import math
import random
import time

from bst import BST
from avl import AVL
from rn import ArbolRN


def cotas_teoricas(n):
    """Devuelve (cota_avl, cota_rn) segun las formulas de la guia."""
    cota_avl = round(1.44 * math.log2(n + 2), 1)
    cota_rn = round(2 * math.log2(n + 1), 1)
    return cota_avl, cota_rn


def insertar_todo(arbol, claves):
    """Inserta todas las claves y devuelve el tiempo empleado en segundos."""
    t0 = time.perf_counter()
    for k in claves:
        arbol.insertar(k, k)
    return time.perf_counter() - t0


def experimento(n, filas):
    """Corre los 3 escenarios para un n dado. Agrega una fila por escenario a "filas"."""
    claves_rnd = random.sample(range(1, n * 10), n)       # n claves distintas al azar
    claves_asc = list(range(1, n + 1))                    # 1..n
    claves_desc = list(range(n, 0, -1))                   # n..1
    cota_avl, cota_rn = cotas_teoricas(n)

    escenarios = [("aleatorio", claves_rnd),
                  ("creciente", claves_asc),
                  ("decreciente", claves_desc)]

    for nombre, claves in escenarios:
        bst, avl, rn = BST(), AVL(), ArbolRN()            # tres arboles vacios

        t_bst = insertar_todo(bst, claves)
        t_avl = insertar_todo(avl, claves)
        t_rn = insertar_todo(rn, claves)

        # --- autocomprobacion: los arboles deben ser correctos ---
        assert avl.es_valido(), "AVL invalido"
        ok_rn, motivo = rn.es_valido()
        assert ok_rn, f"RN invalido: {motivo}"

        h_bst, h_avl, h_rn = bst.altura(), avl.altura(), rn.altura()
        cumple_avl = h_avl <= cota_avl
        cumple_rn = (h_rn - 1) <= cota_rn                 # cota RN esta en aristas

        print(f"n={n:6d} {nombre:12s} | "
              f"BST={h_bst:5d} AVL={h_avl:3d} RN={h_rn:3d} | "
              f"teorico AVL<={cota_avl} RN<={cota_rn} | "
              f"cumple: AVL={'si' if cumple_avl else 'NO'} RN={'si' if cumple_rn else 'NO'}")

        filas.append({
            "n": n, "escenario": nombre,
            "altura_bst": h_bst, "altura_avl": h_avl, "altura_rn": h_rn,
            "cota_avl": cota_avl, "cota_rn": cota_rn,
            "cumple_avl": cumple_avl, "cumple_rn": cumple_rn,
            "t_bst_s": round(t_bst, 4), "t_avl_s": round(t_avl, 4), "t_rn_s": round(t_rn, 4),
            "rn_caso1": rn.estadisticas["caso1"], "rn_caso2": rn.estadisticas["caso2"],
            "rn_caso3": rn.estadisticas["caso3"],
            "rn_rotaciones": rn.estadisticas["rot_izq"] + rn.estadisticas["rot_der"],
        })


def imprimir_tabla_informe(filas):
    """Imprime la tabla de la guia (Actividad 3) con las cotas CORREGIDAS."""
    pedidas = [(100, "aleatorio"), (100, "creciente"), (1000, "aleatorio"),
               (1000, "creciente"), (10000, "creciente")]
    por_clave = {(f["n"], f["escenario"]): f for f in filas}

    print("\nTABLA PARA EL INFORME (formato de la guia, cotas corregidas)")
    print("-" * 86)
    print(f"{'n':>6} | {'Escenario':<11} | {'BST':>5} | {'AVL':>4} | {'RN':>4} | "
          f"{'Cota AVL':>8} | {'Cota RN':>7} | Observacion")
    print("-" * 86)
    for clave in pedidas:
        f = por_clave.get(clave)
        if f is None:                                     # no se ejecuto (modo --rapido)
            continue
        if f["escenario"] == "creciente":
            obs = f"BST degenerado (h = n); RN ~ {f['altura_rn']/math.log2(f['n']+1):.2f}*log2(n+1)"
        else:
            obs = "los tres cerca de log2(n)"
        print(f"{f['n']:>6} | {f['escenario']:<11} | {f['altura_bst']:>5} | {f['altura_avl']:>4} | "
              f"{f['altura_rn']:>4} | {f['cota_avl']:>8} | {f['cota_rn']:>7} | {obs}")
    print("-" * 86)


def guardar_csv(filas, ruta="resultados_experimento.csv"):
    """Guarda todas las filas en un CSV (delimitador coma, UTF-8)."""
    with open(ruta, "w", newline="", encoding="utf-8") as archivo:
        escritor = csv.DictWriter(archivo, fieldnames=list(filas[0].keys()))
        escritor.writeheader()
        escritor.writerows(filas)
    print(f"\nResultados completos guardados en: {ruta}")


def main():
    parser = argparse.ArgumentParser(description="Altura BST vs AVL vs RN")
    parser.add_argument("--rapido", action="store_true",
                        help="solo n = 100, 500, 1000 (el modo completo tarda unos segundos por el BST degenerado)")
    args = parser.parse_args()

    tamanos = [100, 500, 1000] if args.rapido else [100, 500, 1000, 5000, 10000]

    random.seed(2026)                                     # semilla fija -> resultados reproducibles

    print("n        escenario    | alturas (nodos)              | cotas teoricas")
    print("-" * 86)

    filas = []
    inicio = time.perf_counter()
    for n in tamanos:
        experimento(n, filas)
    print(f"\nTiempo total del experimento: {time.perf_counter() - inicio:.1f} s")

    imprimir_tabla_informe(filas)
    guardar_csv(filas)


if __name__ == "__main__":
    main()
