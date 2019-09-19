#include "perfect/cpu_cache.hpp"

int main(void) {

    using namespace perfect;

    int *a = new int[1024];
    flush_all(a, 1024 * sizeof(int));

    // do things with `a` flushed from cache into main memory
    // furthermore, all loads and stores before this function call are guaranteed to be complete

    delete[] a;
}