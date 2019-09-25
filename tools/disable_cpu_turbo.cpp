#include <iostream>

#include "perfect/cpu_turbo.hpp"

using namespace perfect;

int main(void) {

  CpuTurboState state;

  perfect::init();

  PERFECT(get_cpu_turbo_state(&state));

  if (!is_turbo_enabled(state)) {
    std::cerr << "cpu turbo already disabled\n";
    exit(EXIT_SUCCESS);
  } else {
    PERFECT(disable_cpu_turbo());
    std::cerr << "disabled cpu turbo\n";
      exit(EXIT_SUCCESS);
  }
}