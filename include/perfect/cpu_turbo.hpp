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

  CpuTurboState() : enabled(false) {}
};

Result get_cpu_turbo_state(CpuTurboState *state) {
  state->enabled = detail::is_turbo_enabled();
  return Result::SUCCESS;
}

inline bool is_turbo_enabled(CpuTurboState state) { return state.enabled; }

Result set_cpu_turbo_state(CpuTurboState state) {
  if (state.enabled) {
    return detail::enable_cpu_turbo();
  } else {
    return detail::disable_cpu_turbo();
  }
}

inline Result disable_cpu_turbo() { return detail::disable_cpu_turbo(); }
inline Result enable_cpu_turbo() { return detail::enable_cpu_turbo(); }

}; // namespace perfect