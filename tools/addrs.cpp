#include <iostream>

int main(void) {
    int a;
    int *b = new int;
    std::cout << "main:  " << uintptr_t(main) << "\n";
    std::cout << "stack: " << uintptr_t(&a) << "\n";
    std::cout << "heap:  " << uintptr_t(b) << "\n";
    delete b;
}