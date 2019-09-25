#include "perfect/os_perf.hpp"

int main(void) {
  PERFECT(perfect::init());

  for (auto cpu : perfect::cpus()) {
    PERFECT(perfect::os_perf_state_maximum(cpu));
  }
}