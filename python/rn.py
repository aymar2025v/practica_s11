"""
rn.py  -  Arbol Rojo-Negro en Python 3.12 (version espejo de rn.h)
SIS210 - Practica Semana 11

Este modulo lo usan:
    - experimento.py  (Actividad 3: comparar alturas BST vs AVL vs RN)
    - siga_rn.py      (Actividad 4: SIGA-RN con estudiantes UNAP)

Propiedades de un arbol Rojo-Negro:
    P1. Cada nodo es ROJO o NEGRO.
    P2. La raiz es NEGRA.
    P3. Las hojas NIL (None) se consideran NEGRAS.
    P4. Un nodo ROJO no tiene hijos ROJOS.
    P5. Todos los caminos de un nodo a sus hojas NIL tienen igual cantidad de negros.
Consecuencia: altura <= 2*log2(n+1)  ->  busqueda e insercion en O(log n).

Convenciones (iguales a las del C++):
    - Altura se mide en NODOS: arbol vacio = 0, un solo nodo = 1.
    - Altura negra bh(n): negros desde n hasta una hoja NIL, SIN contar a n
      y CONTANDO la hoja NIL.
"""

# Constantes de color. Usamos letras para que la impresion sea legible.
ROJO = "R"
NEGRO = "N"


class NodoRN:
    """Nodo del arbol. __slots__ ahorra memoria y acelera el acceso a atributos
    (importante cuando el experimento crea miles de nodos)."""

    __slots__ = ("clave", "valor", "color", "padre", "izq", "der")

    def __init__(self, clave, valor=None):
        self.clave = clave      # clave de ordenamiento
        self.valor = valor      # dato asociado
        self.color = ROJO       # todo nodo nuevo nace ROJO
        self.padre = None       # referencia al padre (None en la raiz)
        self.izq = None         # hijo izquierdo (None = hoja NIL)
        self.der = None         # hijo derecho


class ArbolRN:
    """Arbol Rojo-Negro con insercion + fixup, busqueda e inorden."""

    def __init__(self):
        self.raiz = None                     # raiz del arbol
        self._tam = 0                        # cantidad de nodos
        self.traza = False                   # si es True, imprime los casos del fixup
        # Contadores para verificar contra el trabajo manual de la Actividad 1
        self.estadisticas = {"caso1": 0, "caso2": 0, "caso3": 0,
                             "rot_izq": 0, "rot_der": 0}

    # ------------------------------------------------------------------
    # Rotaciones (solo reacomodan referencias; el orden BST se conserva)
    # ------------------------------------------------------------------
    def _rotar_izquierda(self, x):
        """x->der (y) sube y x baja a su izquierda.

              x                y
             / \\              / \\
            A   y     ==>     x   C
               / \\          / \\
              B   C         A   B
        """
        y = x.der                    # y es el hijo derecho de x (debe existir)
        x.der = y.izq                # el subarbol B pasa a ser hijo derecho de x
        if y.izq is not None:
            y.izq.padre = x          # actualizamos el padre de B
        y.padre = x.padre            # y hereda el padre de x
        if x.padre is None:          # si x era la raiz...
            self.raiz = y            # ...y es la nueva raiz
        elif x is x.padre.izq:       # si x era hijo izquierdo...
            x.padre.izq = y          # ...y ocupa ese lugar
        else:                        # si x era hijo derecho...
            x.padre.der = y          # ...y ocupa ese lugar
        y.izq = x                    # x pasa a ser hijo izquierdo de y
        x.padre = y                  # y es el nuevo padre de x
        self.estadisticas["rot_izq"] += 1

    def _rotar_derecha(self, x):
        """x->izq (y) sube y x baja a su derecha (simetrica a la anterior)."""
        y = x.izq                    # y es el hijo izquierdo de x (debe existir)
        x.izq = y.der                # el subarbol B pasa a ser hijo izquierdo de x
        if y.der is not None:
            y.der.padre = x
        y.padre = x.padre            # y hereda el padre de x
        if x.padre is None:
            self.raiz = y
        elif x is x.padre.der:
            x.padre.der = y
        else:
            x.padre.izq = y
        y.der = x                    # x pasa a ser hijo derecho de y
        x.padre = y
        self.estadisticas["rot_der"] += 1

    # ------------------------------------------------------------------
    # Insercion
    # ------------------------------------------------------------------
    def insertar(self, clave, valor=None):
        """1) insercion BST normal  2) nodo ROJO  3) fixup.
        Si la clave ya existe, solo se actualiza su valor (no se duplica)."""
        # --- 1) descenso BST iterativo para hallar el padre ---
        padre = None
        actual = self.raiz
        while actual is not None:
            padre = actual
            if clave < actual.clave:
                actual = actual.izq
            elif clave > actual.clave:
                actual = actual.der
            else:                            # clave repetida
                actual.valor = valor
                return

        # --- 2) crear nodo ROJO y enlazarlo ---
        z = NodoRN(clave, valor)
        z.padre = padre
        if padre is None:                    # arbol vacio
            self.raiz = z
        elif clave < padre.clave:
            padre.izq = z
        else:
            padre.der = z
        self._tam += 1

        if self.traza:
            print(f"  Insertar {clave} (ROJO)" + ("  -> es la raiz" if padre is None else ""))

        # --- 3) restaurar propiedades RN ---
        self._insertar_fixup(z)

    def _insertar_fixup(self, z):
        """Mientras el padre de z sea ROJO hay violacion de P4.
        Se mira al TIO (hermano del padre):
            Caso 1: tio ROJO                   -> recolorear y subir el problema.
            Caso 2: tio NEGRO, z "triangulo"   -> rotar el padre (lleva a Caso 3).
            Caso 3: tio NEGRO, z "linea"       -> recolorear + rotar el abuelo.
        El tio None cuenta como NEGRO."""
        while z.padre is not None and z.padre.color == ROJO:
            p = z.padre                      # padre (ROJO, por lo tanto no es la raiz)
            g = p.padre                      # abuelo (existe)

            if p is g.izq:
                # ===== el padre es hijo IZQUIERDO del abuelo =====
                tio = g.der
                if tio is not None and tio.color == ROJO:
                    # ---- CASO 1 ----
                    if self.traza:
                        print(f"    Caso 1 (tio ROJO): padre {p.clave} y tio {tio.clave} -> NEGRO; "
                              f"abuelo {g.clave} -> ROJO; el problema sube a {g.clave}")
                    p.color = NEGRO
                    tio.color = NEGRO
                    g.color = ROJO
                    z = g
                    self.estadisticas["caso1"] += 1
                else:
                    if z is p.der:
                        # ---- CASO 2 (triangulo izq-der) ----
                        if self.traza:
                            print(f"    Caso 2 (triangulo, tio NEGRO): rotacion IZQUIERDA sobre el padre {p.clave}")
                        z = p
                        self._rotar_izquierda(z)
                        p = z.padre          # nuevo padre = el nodo que acaba de subir
                        self.estadisticas["caso2"] += 1
                    # ---- CASO 3 (linea izq-izq) ----
                    if self.traza:
                        print(f"    Caso 3 (linea, tio NEGRO): padre {p.clave} -> NEGRO; abuelo {g.clave} -> ROJO; "
                              f"rotacion DERECHA sobre el abuelo {g.clave}")
                    p.color = NEGRO
                    g.color = ROJO
                    self._rotar_derecha(g)
                    self.estadisticas["caso3"] += 1
            else:
                # ===== el padre es hijo DERECHO del abuelo (espejo) =====
                tio = g.izq
                if tio is not None and tio.color == ROJO:
                    # ---- CASO 1 (simetrico) ----
                    if self.traza:
                        print(f"    Caso 1 (tio ROJO): padre {p.clave} y tio {tio.clave} -> NEGRO; "
                              f"abuelo {g.clave} -> ROJO; el problema sube a {g.clave}")
                    p.color = NEGRO
                    tio.color = NEGRO
                    g.color = ROJO
                    z = g
                    self.estadisticas["caso1"] += 1
                else:
                    if z is p.izq:
                        # ---- CASO 2 (triangulo der-izq) ----
                        if self.traza:
                            print(f"    Caso 2 (triangulo, tio NEGRO): rotacion DERECHA sobre el padre {p.clave}")
                        z = p
                        self._rotar_derecha(z)
                        p = z.padre
                        self.estadisticas["caso2"] += 1
                    # ---- CASO 3 (linea der-der) ----
                    if self.traza:
                        print(f"    Caso 3 (linea, tio NEGRO): padre {p.clave} -> NEGRO; abuelo {g.clave} -> ROJO; "
                              f"rotacion IZQUIERDA sobre el abuelo {g.clave}")
                    p.color = NEGRO
                    g.color = ROJO
                    self._rotar_izquierda(g)
                    self.estadisticas["caso3"] += 1

        # P2: la raiz siempre termina NEGRA
        if self.raiz.color == ROJO:
            if self.traza:
                print(f"    Raiz {self.raiz.clave} estaba ROJA -> NEGRA (P2)")
            self.raiz.color = NEGRO

    # ------------------------------------------------------------------
    # Consultas
    # ------------------------------------------------------------------
    def buscar(self, clave):
        """Busqueda BST iterativa. Devuelve el nodo o None. O(log n)."""
        actual = self.raiz
        while actual is not None:
            if clave < actual.clave:
                actual = actual.izq
            elif clave > actual.clave:
                actual = actual.der
            else:
                return actual
        return None

    def __len__(self):
        return self._tam

    def altura(self):
        """Altura en NODOS (vacio = 0). Iterativa con pila: no depende del
        limite de recursion de Python."""
        if self.raiz is None:
            return 0
        maxima = 0
        pila = [(self.raiz, 1)]              # pares (nodo, profundidad en nodos)
        while pila:
            nodo, prof = pila.pop()
            if prof > maxima:
                maxima = prof
            if nodo.izq is not None:
                pila.append((nodo.izq, prof + 1))
            if nodo.der is not None:
                pila.append((nodo.der, prof + 1))
        return maxima

    def _negros_desde(self, n):
        """Negros desde n hasta NIL contando a n (si es negro) y a NIL (=1).
        Devuelve -1 si hay caminos con distinta cantidad de negros."""
        if n is None:
            return 1
        bi = self._negros_desde(n.izq)
        bd = self._negros_desde(n.der)
        if bi < 0 or bd < 0 or bi != bd:
            return -1
        return bi + (1 if n.color == NEGRO else 0)

    def altura_negra(self, n=None):
        """bh(n): negros hasta NIL sin contar n. Por defecto usa la raiz.
        Devuelve -1 si el arbol es inconsistente."""
        if n is None:
            n = self.raiz
        if n is None:
            return 0
        total = self._negros_desde(n)
        if total < 0:
            return -1
        return total - (1 if n.color == NEGRO else 0)

    def inorden(self, f):
        """Recorrido inorden (izq, nodo, der): llama f(clave, valor) en orden
        ascendente. Version ITERATIVA con pila explicita."""
        pila = []
        actual = self.raiz
        while pila or actual is not None:
            while actual is not None:        # bajar todo lo posible por la izquierda
                pila.append(actual)
                actual = actual.izq
            actual = pila.pop()              # el mas a la izquierda sin visitar
            f(actual.clave, actual.valor)    # visitar
            actual = actual.der              # continuar por la derecha

    # ------------------------------------------------------------------
    # Validacion y visualizacion
    # ------------------------------------------------------------------
    def es_valido(self):
        """Verifica TODAS las propiedades RN, el orden BST y los punteros padre.
        Devuelve (True, "") o (False, motivo)."""
        if self.raiz is None:
            return True, ""
        if self.raiz.color != NEGRO:
            return False, "la raiz no es NEGRA (P2)"
        if self.raiz.padre is not None:
            return False, "la raiz tiene padre"

        motivo = []

        def validar(n, minimo, maximo):
            """Devuelve negros (con NIL) o -1; deja el motivo en la lista."""
            if n is None:
                return 1
            if minimo is not None and not (minimo < n.clave):
                motivo.append(f"orden BST roto en {n.clave}")
                return -1
            if maximo is not None and not (n.clave < maximo):
                motivo.append(f"orden BST roto en {n.clave}")
                return -1
            for hijo in (n.izq, n.der):
                if hijo is not None and hijo.padre is not n:
                    motivo.append(f"puntero padre incorrecto bajo {n.clave}")
                    return -1
            if n.color == ROJO:
                for hijo in (n.izq, n.der):
                    if hijo is not None and hijo.color == ROJO:
                        motivo.append(f"rojo-rojo en {n.clave}")
                        return -1
            bi = validar(n.izq, minimo, n.clave)
            if bi < 0:
                return -1
            bd = validar(n.der, n.clave, maximo)
            if bd < 0:
                return -1
            if bi != bd:
                motivo.append(f"alturas negras distintas en {n.clave}")
                return -1
            return bi + (1 if n.color == NEGRO else 0)

        ok = validar(self.raiz, None, None) >= 0
        return ok, ("" if ok else motivo[0])

    def en_linea(self):
        """Notacion lineal: clave(color)[hijo_izq,hijo_der]  ('-' = NIL)."""
        def rec(n):
            if n is None:
                return "-"
            texto = f"{n.clave}({n.color})"
            if n.izq is not None or n.der is not None:
                texto += f"[{rec(n.izq)},{rec(n.der)}]"
            return texto
        return rec(self.raiz)

    def imprimir(self):
        """Dibujo horizontal: la derecha va ARRIBA y la izquierda ABAJO."""
        def rec(n, nivel):
            if n is None:
                return
            rec(n.der, nivel + 1)
            print(" " * (6 * nivel) + f"{n.clave}({n.color})")
            rec(n.izq, nivel + 1)
        if self.raiz is None:
            print("  (arbol vacio)")
        else:
            rec(self.raiz, 0)
