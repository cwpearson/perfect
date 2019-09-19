#pragma once

#include <nvml.h>

namespace perfect {

/*! initialize the benchmark
 */
Result init() {
  static bool init_ = false;
  if (init_)
    return Result::SUCCESS;

  // init nvml
  nvmlReturn_t ret = nvmlInit();
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }

  // don't init again if init() called twice
  init_ = true;
  return Result::SUCCESS;
}

}; // namespace perfect