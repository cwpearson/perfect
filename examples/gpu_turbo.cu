#include "perfect/gpu_turbo.hpp"
#include "perfect/init.hpp"

#define OR_DIE(expr)

int main(void) {

  using namespace perfect;
  GpuTurboState state;

  init();

  for (unsigned int gpu = 0; gpu < 1; ++gpu) {
    PERFECT(perfect::get_gpu_turbo_state(&state, gpu));
    PERFECT(perfect::disable_gpu_turbo(gpu));
    PERFECT(perfect::set_gpu_turbo_state(state, gpu));
  }
}