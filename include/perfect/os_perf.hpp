#pragma once



#include <vector>
#include <string>
#include <cassert>



#ifdef __linux__
#include "detail/os/linux.hpp"
#else
#error "unsupported platform"
#endif

#include "result.hpp"

namespace perfect {

struct OsPerfState {
#ifdef __linux__
    std::string governor;
#else
#error "unsupported platform"
#endif
};

Result get_os_perf_state(OsPerfState *state, const int cpu) {
    assert(state);
    #ifdef __linux__
    return get_governor(state->governor, cpu);
    #else
    #error "unsupported platform"
    #endif
}

Result os_perf_state_maximum(const int cpu) {
    #ifdef __linux__
    return set_governor(cpu, "performance");
    #else
    #error "unsupported platform"
    #endif
}

Result set_os_perf_state(const int cpu, OsPerfState state) {
        #ifdef __linux__
    return set_governor(cpu, state.governor);
    #else
    #error "unsupported platform"
    #endif

}

};