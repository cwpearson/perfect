#include <iostream>

int main(void) {
    int *a = new int;
    std::cout << "main:  " << uintptr_t(main) << "\n";
    std::cout << "stack: " << uintptr_t(&a) << "\n";
    std::cout << "heap:  " << uintptr_t(a) << "\n";
    delete a;
}