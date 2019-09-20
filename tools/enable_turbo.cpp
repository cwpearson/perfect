#include <iostream>

#include "perfect/cpu_turbo.hpp"

using namespace perfect;

int main(void) {

  Result ret;
  CpuTurboState state;

  perfect::init();

  PERFECT(get_cpu_turbo_state(&state));

  if (is_turbo_enabled(state)) {
    std::cerr << "cpu turbo already enabled\n";
    exit(EXIT_SUCCESS);
  } else {
    PERFECT(enable_cpu_turbo());
    std::cerr << "enabled cpu turbo\n";
      exit(EXIT_SUCCESS);
  }
}