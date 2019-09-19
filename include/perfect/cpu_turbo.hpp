#pragma once

#ifdef __linux__ // linux
#include "detail/os/linux.hpp"

#ifdef __amd64__
#include "detail/turbo/linux_amd64.hpp"
#elif __powerpc64__
#include "detail/turbo/linux_power.hpp"
#else
#error "unsupported CPU arch"
#endif

#else // not linux
#error "unsupported OS"
#endif



#include "result.hpp"

namespace perfect {

struct CpuTurboState {
    bool enabled;
};


Result get_cpu_turbo_state(CpuTurboState *state) {
    state->enabled = is_turbo_enabled();
}

Result set_cpu_turbo_state(CpuTurboState state) {
    if (state.enabled) {
        enable_cpu_turbo();
    } else {
        disable_cpu_turbo();
    }
}

};