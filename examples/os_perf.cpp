#include "perfect/os_perf.hpp"

#include <map>

int main(void) {


    std::map<int, perfect::OsPerfState> states;

    for (auto cpu : perfect::cpus()) {
        perfect::OsPerfState state;
        perfect::get_os_perf_state(&state, cpu);
        states[cpu] = state;
        perfect::os_perf_state_maximum(cpu);
    }

    // do things with all CPUs set to the maximum performancem mode by the OS

    for (auto kv : states) {
        int cpu = kv.first;
        perfect::OsPerfState state = kv.second;
        perfect::set_os_perf_state(cpu, state);
    }
    

}