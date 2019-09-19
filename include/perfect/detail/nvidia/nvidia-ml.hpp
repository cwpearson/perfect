#pragma once

#include <cstdio>
#include <vector>

#include <nvml.h>

#include "perfect/result.hpp"

inline void checkNvml(nvmlReturn_t result, const char *file, const int line) {
  if (result != NVML_SUCCESS) {
    printf("%s@%d: NVML Error: %s\n", file, line, nvmlErrorString(result));
    exit(-1);
  }
}

#define NVML(stmt) checkNvml(stmt, __FILE__, __LINE__);

namespace perfect {
namespace detail {

Result get_device_memory_clocks(std::vector<unsigned int> &memoryClocksMhz, unsigned int index) {
  nvmlDevice_t device;
  nvmlReturn_t ret;
  ret = nvmlDeviceGetHandleByIndex(index, &device);
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }

  unsigned int resultCount = 0;
  ret = nvmlDeviceGetSupportedMemoryClocks(device, &resultCount, nullptr);
  if (ret != NVML_ERROR_INSUFFICIENT_SIZE) {
    return from_nvml(ret);
  }
  memoryClocksMhz.resize(resultCount);
  NVML(nvmlDeviceGetSupportedMemoryClocks(device, &resultCount, memoryClocksMhz.data()));
  return Result::SUCCESS;
}

Result get_device_graphics_clocks(std::vector<unsigned int> &graphicsClocksMhz,
                                  unsigned int index,
                                  unsigned int memoryClockMhz) {
  nvmlDevice_t device;
  nvmlDeviceGetHandleByIndex(index, &device);
  unsigned int resultCount = 0;
  auto ret = nvmlDeviceGetSupportedGraphicsClocks(device, memoryClockMhz,
                                                  &resultCount, nullptr);
  if (ret != NVML_ERROR_INSUFFICIENT_SIZE) {
    return from_nvml(ret);
  }
  graphicsClocksMhz.resize(resultCount);
  return from_nvml(nvmlDeviceGetSupportedGraphicsClocks(
      device, memoryClockMhz, &resultCount, graphicsClocksMhz.data()));
}

Result disable_gpu_turbo(unsigned int idx) {
  nvmlDevice_t device;
  nvmlReturn_t ret;
  ret = nvmlDeviceGetHandleByIndex(idx, &device);
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }
  return from_nvml(
      nvmlDeviceSetAutoBoostedClocksEnabled(device, NVML_FEATURE_DISABLED));
}

Result enable_gpu_turbo(unsigned int idx) {
  nvmlDevice_t device;
  nvmlReturn_t ret;
  ret = nvmlDeviceGetHandleByIndex(idx, &device);
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }
  return from_nvml(
      nvmlDeviceSetAutoBoostedClocksEnabled(device, NVML_FEATURE_ENABLED));
}

Result is_gpu_turbo_enabled(bool *enabled, unsigned int idx) {
  nvmlDevice_t device;
  nvmlReturn_t ret;
  nvmlEnableState_t isEnabled;
  nvmlEnableState_t defaultIsEnabled;
  ret = nvmlDeviceGetHandleByIndex(idx, &device);
  if (ret != NVML_SUCCESS) {
    return from_nvml(ret);
  }
  ret = nvmlDeviceGetAutoBoostedClocksEnabled(device, &isEnabled,
                                              &defaultIsEnabled);

  *enabled = (isEnabled == NVML_FEATURE_ENABLED);
  return from_nvml(ret);
}

} // namespace detail
} // namespace perfect