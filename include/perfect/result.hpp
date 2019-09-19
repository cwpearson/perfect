#pragma once

#include <cassert>

#include <nvml.h>

namespace perfect {
enum class Result {
  SUCCESS,
  NVML_NOT_SUPPORTED,
  NVML_NO_PERMISSION,
  NVML_UNINITIALIZED,
  NO_PERMISSION,
  UNKNOWN
};





Result from_nvml(nvmlReturn_t nvml) {
  switch (nvml) {
  case NVML_SUCCESS:
    return Result::SUCCESS;
  case NVML_ERROR_UNINITIALIZED:
    return Result::NVML_UNINITIALIZED;
  case NVML_ERROR_NOT_SUPPORTED:
    return Result::NVML_NOT_SUPPORTED;
  case NVML_ERROR_INVALID_ARGUMENT:
  case NVML_ERROR_GPU_IS_LOST:
  case NVML_ERROR_UNKNOWN:
  default:
    assert(0 && "unhandled nvmlReturn_t");
  }
  return Result::UNKNOWN;
}

const char *get_string(const Result &result) {
  switch (result) {
  case Result::SUCCESS:
    return "success";
  case Result::NO_PERMISSION:
    return "no permission";
  case Result::UNKNOWN:
    return "unknown error";
  case Result::NVML_NOT_SUPPORTED:
    return "nvidia-ml returned not supported";
  case Result::NVML_NO_PERMISSION:
    return "nvidia-ml returned no permission";
  default:
    assert(0 && "unexpected perfect::Result");
  }

  assert(0 && "unreachable");
  return "";
}

inline void check(Result result, const char *file, const int line) {
  if (result != Result::SUCCESS) {
    fprintf(stderr, "%s@%d: perfect Error: %s\n", file, line,
           get_string(result));
    exit(-1);
  }
}

} // namespace perfect

#define PERFECT(stmt) check(stmt, __FILE__, __LINE__);
