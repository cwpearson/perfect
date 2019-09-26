#pragma once

#include <vector>
#include <string>
#include <cassert>
#include <map>

#ifdef __linux__
#include "detail/os/linux.hpp"
#else
#error "unsupported platform"
#endif

#include "result.hpp"
#include "init.hpp"

namespace perfect {

struct OsPerfState {
#ifdef __linux__
    std::map<int, std::string> governors;
#else
#error "unsupported platform"
#endif
};

Result get_os_perf_state(OsPerfState &state) {
    #ifdef __linux__
    for (auto cpu : cpus()) {
        std::string gov;
        PERFECT_SUCCESS_OR_RETURN(get_governor(gov, cpu));
        state.governors[cpu] = gov;
    }
    #else
    #error "unsupported platform"
    #endif
    return Result::SUCCESS;
}

Result os_perf_state_maximum(const int cpu) {
    #ifdef __linux__
    return set_governor(cpu, "performance");
    #else
    #error "unsupported platform"
    #endif
}

Result os_perf_state_minimum(const int cpu) {
    #ifdef __linux__
    return set_governor(cpu, "powersave");
    #else
    #error "unsupported platform"
    #endif
}

Result set_os_perf_state(OsPerfState state) {
    #ifdef __linux__
    for (auto kv : state.governors) {
        PERFECT_SUCCESS_OR_RETURN(set_governor(kv.first, kv.second));
    }
    #else
    #error "unsupported platform"
    #endif
    return Result::SUCCESS;
}

};