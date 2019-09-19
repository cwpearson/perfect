#pragma once

#include "detail/nvidia/nvidia-ml.hpp"

namespace perfect {

/*!
 */
Result set_max_gpu_clocks(unsigned int idx) {

  Result rt;
  std::vector<unsigned int> clksMhz;

  ret = get_device_memory_clocks(clksMhz, idx);

  auto maxMemMhz = *std::max_element(memClksMhz.begin(), memClksMhz.end());
  ret = get_device_graphics_clocks(clksMhz, idx);
  auto maxCoreMhz = *std::max_element(clksMhz.begin(), clksMhz.end());

  auto ret = nvmlDeviceSetApplicationsClocks(device, maxMemMhz, maxCoreMhz);
  if (ret == NVML_ERROR_NOT_SUPPORTED) {
    return Result::NVML_NOT_SUPPORTED;
  } else if (ret == NVML_ERROR_NO_PERMISSION) {
    return Result::NVML_NO_PERMISSION;
  }
  return Result::SUCCESS;
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
  ret = nvmlDeviceResetApplicationsClocks(device);
  if (ret == NVML_ERROR_NOT_SUPPORTED) {
    return Result::NVML_NOT_SUPPORTED;
  } else if (ret == NVML_ERROR_NO_PERMISSION) {
    return Result::NVML_NO_PERMISSION;
  }
  return Result::SUCCESS;
}

}; // namespace perfect