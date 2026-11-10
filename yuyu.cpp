#include <iostream>
#include <memory>
#include <utility>

template<typename T>
struct Nodo{
    T dato;
    std::unique_ptr<Nodo<T>> izq, der;
    Nodo(T d): dato(std::move(d)), izq(nullptr), der(nullptr) {}
};

int main(){

    int dato = 5;
    std::unique_ptr<Nodo<int>> raiz = std::make_unique<Nodo<int>>(dato);
    std::cout << raiz->dato;

    return 0;
}