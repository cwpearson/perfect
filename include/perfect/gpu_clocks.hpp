#pragma once

#include <algorithm>

#include "detail/nvidia/nvidia-ml.hpp"

#include "result.hpp"
#include "init.hpp"

namespace perfect {

/*!
 */
Result set_max_gpu_clocks(unsigned int idx) {

  Result ret;
  std::vector<unsigned int> clksMhz;
  nvmlDevice_t device;

  ret = from_nvml(nvmlDeviceGetHandleByIndex(idx, &device));
  if (ret != Result::SUCCESS) {
    return ret;
  }

  ret = detail::get_device_memory_clocks(clksMhz, idx);
    if (ret != Result::SUCCESS) {
    return ret;
    }

  auto maxMemMhz = *std::max_element(clksMhz.begin(), clksMhz.end());
  ret = detail::get_device_graphics_clocks(clksMhz, idx, maxMemMhz);
  if (ret != Result::SUCCESS) {
    return ret;
  }
  auto maxCoreMhz = *std::max_element(clksMhz.begin(), clksMhz.end());
  return from_nvml(
      nvmlDeviceSetApplicationsClocks(device, maxMemMhz, maxCoreMhz));
}

/*! Reset GPU clocks to default behavior
 */
Result reset_gpu_clocks(unsigned int idx) {

  nvmlDevice_t device;
  nvmlReturn_t ret;
  ret = nvmlDeviceGetHandleByIndex(idx, &device);
  if (ret != NVML_SUCCESS) {
    assert(false);
  }
  return from_nvml(nvmlDeviceResetApplicationsClocks(device));
}

}; // namespace perfect