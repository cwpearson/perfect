#pragma once

#ifdef __NVCC__
#define PERFECT_HAS_CUDA
#endif

#ifdef PERFECT_HAS_CUDA
#include <nvml.h>
#endif

namespace perfect {

/*! initialize the benchmark
 */
Result init() {
  static bool init_ = false;
  if (init_)
    return Result::SUCCESS;

// init nvml
#ifdef PERFECT_HAS_CUDA
  nvmlReturn_t ret = nvmlInit();
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }
#endif

  // don't init again if init() called twice
  init_ = true;
  return Result::SUCCESS;
}

}; // namespace perfect