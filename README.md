# Práctica Semana 11 — Árboles Rojo-Negro (SIS210, UNAP 2026-II)

Todo el código está probado (C++17 con g++ y sanitizers; Python 3.12). Usa solo la
biblioteca estándar: **no hay que instalar nada extra**.

## Estructura

```
practica_s11/
├── README.md
├── ejecutar_todo.bat          <- compila y corre TODO en orden (opcional)
├── cpp/
│   ├── rn.h                   <- Actividad 2: árbol RN genérico (los 4 TODO resueltos)
│   ├── rn.cpp                 <- Actividades 1 y 2: pruebas P1-P4, Secuencias A y B, estrés
│   ├── actividad1_traza.cpp   <- Actividad 1: inserciones paso a paso (para comparar tus dibujos)
│   └── compilar.bat           <- compila los dos programas con g++
└── python/
    ├── rn.py                  <- árbol RN en Python (espejo del C++)
    ├── bst.py                 <- BST sin balanceo (iterativo)
    ├── avl.py                 <- AVL
    ├── pruebas_rn.py          <- pruebas de rn.py / bst.py / avl.py
    ├── experimento.py         <- Actividad 3: BST vs AVL vs RN
    └── siga_rn.py             <- Actividad 4: SIGA-RN con estudiantes UNAP
```

## Cómo ejecutar (terminal de VS Code en Windows)

### C++ (necesitas `g++` con soporte C++17: MinGW-w64 / MSYS2)

```powershell
cd cpp
g++ -std=c++17 -Wall -Wextra -O2 rn.cpp -o rn.exe
.\rn.exe

g++ -std=c++17 -Wall -Wextra -O2 actividad1_traza.cpp -o actividad1_traza.exe
.\actividad1_traza.exe
```

O con el script: `.\compilar.bat` y luego `.\rn.exe` / `.\actividad1_traza.exe`.

*Si usas Visual Studio (MSVC) en vez de g++*, abre la "Developer PowerShell for VS" y:
`cl /EHsc /std:c++17 /utf-8 rn.cpp` → `.\rn.exe`

### Python 3.12

```powershell
cd python
python pruebas_rn.py          # 1) primero: verifica que las estructuras son correctas
python experimento.py         # 2) Actividad 3 (completo; --rapido para probar con n<=1000)
python siga_rn.py             # 3) Actividad 4 (--rapido omite la prueba de escala)
```

(Si `python` no funciona, prueba `py -3.12 pruebas_rn.py`.)

### Guardar las salidas para el informe

```powershell
.\rn.exe > salida_rn.txt
.\actividad1_traza.exe > salida_traza.txt
python pruebas_rn.py > salida_pruebas_py.txt
python experimento.py > salida_experimento.txt     # además crea resultados_experimento.csv
python siga_rn.py > salida_siga.txt
```

## Orden recomendado de ejecución

| # | Programa | Qué demuestra |
|---|----------|---------------|
| 1 | `rn.exe` | 57 verificaciones: P1–P4, Secuencias A y B, estrés (valida el árbol tras **cada** inserción), duplicados, plantilla con `string` |
| 2 | `actividad1_traza.exe` | Cada paso de las secuencias A y B: caso aplicado (1/2/3), rotación, árbol dibujado, bh |
| 3 | `pruebas_rn.py` | Lo mismo para la versión Python + BST + AVL |
| 4 | `experimento.py` | Alturas BST/AVL/RN en 5 tamaños × 3 escenarios, tabla del informe y CSV |
| 5 | `siga_rn.py` | 10 estudiantes UNAP, búsquedas cronometradas, listado ordenado, prueba de escala |

## Correcciones respecto a la guía (anótalas en el informe)

1. **Secuencia A — criterio de corrección:** el árbol correcto es `20(N)[10(N)[-,15(R)], 30(N)[25(R),-]]`
   (25 es ROJO, hijo de 30; la guía decía 25(N) como hijo de 20).
2. **Caso de prueba P2 (10, 20, 30):** es el **Caso 3** (tío nulo = negro → una rotación izquierda),
   no el Caso 1. Las aserciones de la guía (raíz = 20, bh = 1) sí se cumplen.
3. **Secuencia B (7, 3, 18, 10, 22, 8, 11, 26):** solo ocurre el **Caso 1** (2 veces) y **no hay
   rotaciones**. Al agregar la clave **15** se activan los tres casos (1, 2 y 3) en una sola inserción.
4. **Tabla de la Actividad 3:** las cotas de AVL (23, 30, 37) no coinciden con la fórmula del script
   (1.44·log₂(n+2)); los valores correctos son 9.6, 14.4 y 19.1 (n = 100, 1000, 10000).
5. **Altura negra:** se usa la convención de CLRS (no se cuenta el nodo, **sí** la hoja NIL). Es la
   única con la que los asserts de la guía (bh = 1 y bh = 2) son verdaderos.
6. **Un solo lenguaje por actividad:** la guía pedía `from rn import ArbolRN` (Python) pero la
   implementación era C++. Aquí hay una versión Python (`rn.py`) equivalente a `rn.h`.

## Mapa para exponer `rn.h` línea por línea

1. **Cabecera y propiedades** (P1–P5) → por qué la altura es ≤ 2·log₂(n+1).
2. **`NodoRN`**: `unique_ptr` para los hijos (un solo dueño) y puntero crudo para el padre.
3. **`propietario(n)`**: devuelve el `unique_ptr` que posee a un nodo (clave para las rotaciones).
4. **`rotar_izquierda` / `rotar_derecha`**: los 8 pasos de movimiento de propiedad.
5. **`insertar_fixup`**: Caso 1 (recolorear), Caso 2 (triángulo → rotación), Caso 3 (línea → recolor + rotación) y los espejos.
6. **`altura_negra_impl` + `validar_rec`**: cómo se detecta un árbol inválido (devuelve −1).
7. **`insertar`**: descenso BST → nodo rojo → fixup. Ojo: tras `std::move(clave)` se usa `z->clave`.

## Problemas frecuentes

- *`g++` no se reconoce:* instala MSYS2/MinGW-w64 y agrega su carpeta `bin` al PATH, o usa MSVC.
- *`python` no se reconoce:* usa `py` o instala Python 3.12 desde python.org (marca "Add to PATH").
- *`experimento.py` tarda:* el BST degenerado con n = 10000 hace ~50 millones de pasos; es normal
  (unos segundos). Usa `--rapido` para probar.
- *Tiempos distintos a los de otro compañero:* los **tiempos** dependen de la PC; las **alturas** no
  (usan semilla fija 2026 y deben coincidir).
