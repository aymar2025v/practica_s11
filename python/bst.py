"""
bst.py  -  Arbol binario de busqueda (BST) SIN balanceo
SIS210 - Practica Semana 11 (Actividad 3)

Es la linea base del experimento: no se auto-balancea, asi que con claves en
orden creciente o decreciente se degenera en una LISTA (altura = n).

IMPORTANTE: insercion y altura son ITERATIVAS. Una version recursiva
reventaria con RecursionError (limite de ~1000 llamadas en Python) cuando
n = 5000 o 10000 en orden creciente, porque la profundidad del arbol seria n.
"""


class NodoBST:
    """Nodo minimo del BST (__slots__ para ahorrar memoria)."""

    __slots__ = ("clave", "valor", "izq", "der")

    def __init__(self, clave, valor=None):
        self.clave = clave
        self.valor = valor
        self.izq = None
        self.der = None


class BST:
    """BST simple: solo inserta y mide la altura (lo que pide el experimento)."""

    def __init__(self):
        self.raiz = None
        self._tam = 0

    def insertar(self, k, v=None):
        """Insercion iterativa: baja desde la raiz hasta hallar el hueco."""
        nuevo = NodoBST(k, v)
        if self.raiz is None:                # arbol vacio
            self.raiz = nuevo
            self._tam = 1
            return
        actual = self.raiz
        while True:
            if k < actual.clave:             # ir a la izquierda
                if actual.izq is None:       # hueco encontrado
                    actual.izq = nuevo
                    break
                actual = actual.izq
            elif k > actual.clave:           # ir a la derecha
                if actual.der is None:
                    actual.der = nuevo
                    break
                actual = actual.der
            else:                            # clave repetida: solo actualizar valor
                actual.valor = v
                return
        self._tam += 1

    def altura(self):
        """Altura en NODOS (vacio = 0). Iterativa con pila de (nodo, profundidad)."""
        if self.raiz is None:
            return 0
        maxima = 0
        pila = [(self.raiz, 1)]
        while pila:
            nodo, prof = pila.pop()
            if prof > maxima:
                maxima = prof
            if nodo.izq is not None:
                pila.append((nodo.izq, prof + 1))
            if nodo.der is not None:
                pila.append((nodo.der, prof + 1))
        return maxima

    def __len__(self):
        return self._tam
