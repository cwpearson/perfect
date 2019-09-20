#pragma once

#include "detail/nvidia/nvidia-ml.hpp"

#include "result.hpp"
#include "init.hpp"

namespace perfect {

struct GpuTurboState {
  bool enabled;

  GpuTurboState() : enabled(false) {}
};

Result get_gpu_turbo_state(GpuTurboState *state, unsigned int idx) {
  return detail::is_gpu_turbo_enabled(&(state->enabled), idx);
}

inline bool is_turbo_enabled(GpuTurboState state) { return state.enabled; }

Result set_gpu_turbo_state(GpuTurboState state, unsigned int idx) {
  if (state.enabled) {
    return detail::enable_gpu_turbo(idx);
  } else {
    return detail::disable_gpu_turbo(idx);
  }
}

inline Result disable_gpu_turbo(unsigned int idx) {
  return detail::disable_gpu_turbo(idx);
}
inline Result enable_gpu_turbo(unsigned int idx) {
  return detail::enable_gpu_turbo(idx);
}

}; // namespace perfect