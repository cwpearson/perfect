#include <iostream>

#include "perfect/cpu_turbo.hpp"
#include "perfect/init.hpp"

using namespace perfect;

int main(void) {

  Result ret;
  CpuTurboState state;

  perfect::init();

  ret = get_cpu_turbo_state(&state);

  if (ret != Result::SUCCESS) {
    std::cerr << "ERROR: " << get_string(ret) << "\n";
    exit(EXIT_FAILURE);
  }

  if (is_turbo_enabled(state)) {
    std::cerr << "turbo already enabled\n";
    exit(EXIT_SUCCESS);
  } else {
    ret = enable_cpu_turbo();
    if (ret != Result::SUCCESS) {
      std::cerr << "ERROR: " << get_string(ret) << "\n";
      exit(EXIT_FAILURE);
    } else {
      std::cerr << "enabled turbo\n";
      exit(EXIT_SUCCESS);
    }
  }
}