"""
siga_rn.py  -  Actividad 4: SIGA-RN (gestion de estudiantes UNAP con Rojo-Negro)
SIS210 - Practica Semana 11

Ejecutar (terminal de VS Code, Windows):
    python siga_rn.py             -> prueba con 10 estudiantes + prueba de escala
    python siga_rn.py --rapido    -> solo la prueba con los 10 estudiantes

Que hace:
    1. Matricula 10 estudiantes (la clave del arbol es el CODIGO).
    2. Busca dos codigos midiendo el tiempo en nanosegundos (como pide la guia).
    3. Lista los estudiantes ordenados por codigo (recorrido inorden).
    4. Extra: repite la busqueda muchas veces para obtener un tiempo promedio
       FIABLE y muestra como crece la altura con n (crecimiento logaritmico).

Nota: una sola busqueda en un arbol de 10 nodos dura apenas unos cientos de
nanosegundos y esa medida tiene mucho ruido (cache, planificador del SO).
Por eso se agrega el promedio de miles de repeticiones.
"""
import argparse
import random
import time
from dataclasses import dataclass

from rn import ArbolRN   # nuestra implementacion Rojo-Negro (archivo rn.py)


@dataclass
class EstudianteUNAP:
    """Registro de un estudiante. dataclass genera __init__, __repr__, etc."""
    codigo: str        # formato PU-XXXXXXXX (PU- + 8 digitos)
    nombres: str
    escuela: str
    ciclo: int
    promedio: float


class SIGA_RN:
    """Sistema de gestion de estudiantes basado en un arbol Rojo-Negro."""

    def __init__(self):
        self.arbol = ArbolRN()              # clave = codigo, valor = EstudianteUNAP

    def matricular(self, est: EstudianteUNAP):
        """Inserta al estudiante usando su codigo como clave. O(log n)."""
        self.arbol.insertar(est.codigo, est)

    def buscar(self, codigo: str):
        """Busca un codigo, imprime el tiempo (ns) y la altura del arbol."""
        t0 = time.perf_counter_ns()                       # tiempo antes
        nodo = self.arbol.buscar(codigo)                  # busqueda O(log n)
        t1 = time.perf_counter_ns()                       # tiempo despues
        print(f"Busqueda '{codigo}': {t1 - t0} ns, "
              f"altura arbol = {self.arbol.altura()}")
        return nodo.valor if nodo else None               # estudiante o None

    def listar_ordenado(self):
        """Recorrido inorden -> lista ordenada por codigo."""
        resultado = []
        self.arbol.inorden(lambda k, v: resultado.append(v))
        return resultado

    def medir_busqueda(self, codigo: str, repeticiones: int = 100_000):
        """Tiempo PROMEDIO (ns) de una busqueda, repitiendola muchas veces."""
        buscar = self.arbol.buscar                        # referencia local (evita lookup repetido)
        t0 = time.perf_counter_ns()
        for _ in range(repeticiones):
            buscar(codigo)
        t1 = time.perf_counter_ns()
        return (t1 - t0) / repeticiones


def cargar_diez_estudiantes():
    """Los 10 estudiantes de la guia (en el MISMO orden de matricula)."""
    return [
        EstudianteUNAP("PU-20200101", "Mamani Quispe, J.", "Sistemas", 5, 14.2),
        EstudianteUNAP("PU-20200387", "Huanca Apaza, L.", "Sistemas", 5, 13.8),
        EstudianteUNAP("PU-20210045", "Ccopa Flores, M.", "IACD", 3, 15.1),
        EstudianteUNAP("PU-20210312", "Ticona Condori, R.", "Sistemas", 3, 12.9),
        EstudianteUNAP("PU-20220089", "Cutipa Larico, A.", "IACD", 1, 16.0),
        EstudianteUNAP("PU-20220154", "Pari Calisaya, Y.", "Sistemas", 1, 11.5),
        EstudianteUNAP("PU-20180023", "Coyla Mamani, H.", "Sistemas", 9, 15.7),
        EstudianteUNAP("PU-20190278", "Quispe Tito, B.", "IACD", 7, 14.8),
        EstudianteUNAP("PU-20230012", "Apaza Cruz, N.", "Sistemas", 1, 13.3),
        EstudianteUNAP("PU-20230198", "Larico Huanca, C.", "IACD", 1, 12.1),
    ]


def prueba_diez_estudiantes():
    """Parte principal de la Actividad 4."""
    print("=" * 66)
    print("PARTE 1: SIGA-RN con 10 estudiantes UNAP")
    print("=" * 66)

    siga = SIGA_RN()
    for e in cargar_diez_estudiantes():
        siga.matricular(e)

    # --- estado del arbol ---
    ok, motivo = siga.arbol.es_valido()
    print(f"\nEstudiantes matriculados : {len(siga.arbol)}")
    print(f"Arbol valido (RN)        : {'SI' if ok else 'NO -> ' + motivo}")
    print(f"Altura del arbol (nodos) : {siga.arbol.altura()}")
    print(f"Altura negra (bh) raiz   : {siga.arbol.altura_negra()}")
    print(f"Raiz                     : {siga.arbol.raiz.clave} ({siga.arbol.raiz.color})")
    st = siga.arbol.estadisticas
    print(f"Fixup                    : Caso1={st['caso1']} Caso2={st['caso2']} Caso3={st['caso3']} "
          f"| rotaciones izq={st['rot_izq']} der={st['rot_der']}")
    print("\nArbol (derecha arriba, izquierda abajo):")
    siga.arbol.imprimir()

    # --- busquedas (las dos de la guia + una que no existe) ---
    print("\nBusquedas individuales (tiempo de UNA ejecucion, con ruido):")
    e1 = siga.buscar("PU-20200387")
    e2 = siga.buscar("PU-20230198")
    e3 = siga.buscar("PU-99999999")                       # no existe -> None
    print(f"  -> {e1.nombres if e1 else None} | {e2.nombres if e2 else None} | {e3}")

    # --- tiempo promedio fiable ---
    print("\nTiempo PROMEDIO de busqueda (100 000 repeticiones):")
    for codigo in ("PU-20200387", "PU-20230198", "PU-99999999"):
        promedio = siga.medir_busqueda(codigo)
        print(f"  '{codigo}': {promedio:8.1f} ns por busqueda")

    # --- listado ordenado ---
    print("\nLista ordenada (inorden):")
    for e in siga.listar_ordenado():
        print(f"  {e.codigo} | {e.nombres} | ciclo {e.ciclo}")

    # --- verificacion: el inorden debe estar realmente ordenado ---
    codigos = [e.codigo for e in siga.listar_ordenado()]
    assert codigos == sorted(codigos), "El listado no esta ordenado"
    print("\nVerificacion: el listado esta ordenado por codigo -> OK")


def prueba_escala():
    """Extra: muestra que la altura y el tiempo de busqueda crecen como log(n)."""
    print("\n" + "=" * 66)
    print("PARTE 2 (extra): crecimiento con el numero de estudiantes")
    print("=" * 66)
    print(f"{'n':>8} | {'altura':>6} | {'bh':>3} | {'ns/busqueda (prom.)':>20}")
    print("-" * 48)

    random.seed(2026)
    for n in (10, 100, 1_000, 10_000, 100_000):
        siga = SIGA_RN()
        # Codigos sinteticos PU-AAAANNNN (8 digitos), todos distintos
        numeros = random.sample(range(2015_0000, 2025_9999), n)
        for num in numeros:
            siga.matricular(EstudianteUNAP(f"PU-{num}", f"Alumno {num}", "Sistemas", 1, 10.0))
        objetivo = f"PU-{numeros[n // 2]}"                # un codigo que SI existe
        promedio = siga.medir_busqueda(objetivo, repeticiones=50_000)
        print(f"{n:>8} | {siga.arbol.altura():>6} | {siga.arbol.altura_negra():>3} | {promedio:>20.1f}")
    print("\n(La altura crece ~log2(n): 10 veces mas datos solo suma ~3 niveles.)")


def main():
    parser = argparse.ArgumentParser(description="SIGA-RN")
    parser.add_argument("--rapido", action="store_true", help="omite la prueba de escala")
    args = parser.parse_args()

    prueba_diez_estudiantes()
    if not args.rapido:
        prueba_escala()


if __name__ == "__main__":
    main()
