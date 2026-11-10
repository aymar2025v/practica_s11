"""
avl.py  -  Arbol AVL (autobalanceado por altura)
SIS210 - Practica Semana 11 (Actividad 3)

(La guia dice "tu implementacion de S10"; aqui va una implementacion completa
por si hace falta. Si ya tienes tu AVL de la Semana 10, puedes reemplazar este
archivo siempre que exponga:  insertar(k, v)  y  altura().)

Regla AVL: en cada nodo |altura(izq) - altura(der)| <= 1.
Si al insertar se rompe, se corrige con rotaciones:
    LL -> rotacion derecha          RR -> rotacion izquierda
    LR -> izquierda en hijo + derecha     RL -> derecha en hijo + izquierda
Cota teorica de altura:  h <= 1.44 * log2(n + 2)   (mas estricta que la de RN).

La recursion es segura: la profundidad nunca pasa de ~1.44*log2(n) (< 20 para
n = 10000), muy lejos del limite de Python.
"""


class NodoAVL:
    __slots__ = ("clave", "valor", "izq", "der", "altura")

    def __init__(self, clave, valor=None):
        self.clave = clave
        self.valor = valor
        self.izq = None
        self.der = None
        self.altura = 1          # un nodo solo tiene altura 1 (en nodos)


class AVL:
    def __init__(self):
        self.raiz = None
        self._tam = 0

    # ---- utilidades de altura y balance ----
    @staticmethod
    def _alt(n):
        """Altura de un subarbol (None -> 0)."""
        return n.altura if n is not None else 0

    def _actualizar(self, n):
        """Recalcula la altura de n a partir de la de sus hijos."""
        n.altura = 1 + max(self._alt(n.izq), self._alt(n.der))

    def _balance(self, n):
        """Factor de balance = altura(izq) - altura(der). Valido: -1, 0, 1."""
        return self._alt(n.izq) - self._alt(n.der)

    # ---- rotaciones (devuelven la nueva raiz del subarbol) ----
    def _rot_der(self, y):
        x = y.izq                # x sube
        y.izq = x.der            # el subarbol der de x pasa a ser izq de y
        x.der = y                # y baja a la derecha de x
        self._actualizar(y)      # primero el que quedo abajo...
        self._actualizar(x)      # ...luego el que quedo arriba
        return x

    def _rot_izq(self, x):
        y = x.der                # y sube
        x.der = y.izq
        y.izq = x
        self._actualizar(x)
        self._actualizar(y)
        return y

    # ---- insercion recursiva ----
    def _insertar(self, n, k, v):
        if n is None:                         # hueco: crear nodo
            self._tam += 1
            return NodoAVL(k, v)
        if k < n.clave:
            n.izq = self._insertar(n.izq, k, v)
        elif k > n.clave:
            n.der = self._insertar(n.der, k, v)
        else:                                 # repetida: actualizar valor
            n.valor = v
            return n

        self._actualizar(n)                   # actualizar altura al volver de la recursion
        b = self._balance(n)

        if b > 1:                             # desbalanceado a la izquierda
            if self._balance(n.izq) < 0:      # caso LR
                n.izq = self._rot_izq(n.izq)
            return self._rot_der(n)           # caso LL (o LR ya convertido)
        if b < -1:                            # desbalanceado a la derecha
            if self._balance(n.der) > 0:      # caso RL
                n.der = self._rot_der(n.der)
            return self._rot_izq(n)           # caso RR (o RL ya convertido)
        return n

    def insertar(self, k, v=None):
        self.raiz = self._insertar(self.raiz, k, v)

    def altura(self):
        """Altura en NODOS (vacio = 0)."""
        return self._alt(self.raiz)

    def __len__(self):
        return self._tam

    def es_valido(self):
        """Comprueba orden BST, alturas guardadas correctas y |balance| <= 1."""
        def rec(n, minimo, maximo):
            if n is None:
                return True
            if minimo is not None and not (minimo < n.clave):
                return False
            if maximo is not None and not (n.clave < maximo):
                return False
            if n.altura != 1 + max(self._alt(n.izq), self._alt(n.der)):
                return False
            if abs(self._balance(n)) > 1:
                return False
            return rec(n.izq, minimo, n.clave) and rec(n.der, n.clave, maximo)
        return rec(self.raiz, None, None)
