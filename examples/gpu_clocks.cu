#include <iostream>

#include "perfect/gpu_clocks.hpp"
#include "perfect/init.hpp"

int main(void) {
    using namespace perfect;
    init();

    for (unsigned int gpu = 0; gpu < 1; ++gpu) {
      PERFECT(perfect::set_max_gpu_clocks(gpu));
      PERFECT(perfect::reset_gpu_clocks(gpu));
    }

    return 0;
}