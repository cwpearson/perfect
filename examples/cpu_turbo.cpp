#include <iostream>

#include "perfect/cpu_turbo.hpp"

int main(void) {

  perfect::Result ret;
  perfect::CpuTurboState state;

  // get the current turbo state
  ret = perfect::get_cpu_turbo_state(&state);
  if (ret != perfect::Result::SUCCESS) {
    std::cerr << perfect::get_string(ret) << "\n";
    exit(EXIT_FAILURE);
  }

  // disable turbo
  if ((ret = perfect::disable_cpu_turbo()) != perfect::Result::SUCCESS) {
    std::cerr << perfect::get_string(ret) << " when disabling turbo\n";
    exit(EXIT_FAILURE);
  }

  // do things with CPU turbo disabled

  // restore the original state
  ret = perfect::set_cpu_turbo_state(state);
}