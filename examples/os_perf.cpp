#include "perfect/os_perf.hpp"

#include <map>

int main(void) {
  perfect::init();

  // os performance state for each cpu
  perfect::OsPerfState state;

  // store the current state
  PERFECT(perfect::get_os_perf_state(state));

  // max state for each cpu
  for (auto cpu : perfect::cpus()) {
    PERFECT(perfect::os_perf_state_maximum(cpu));
  }

  // do things with all CPUs set to the maximum performancem mode by the OS

  // restore original state
  PERFECT(perfect::set_os_perf_state(state));

}