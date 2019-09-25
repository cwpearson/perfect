#pragma once

#include <cassert>
#include <fstream>

#include "perfect/result.hpp"
#include "perfect/detail/fs.hpp"

namespace perfect {
namespace detail {

bool has_intel_pstate_no_turbo() {
  return bool(std::ifstream("/sys/devices/system/cpu/intel_pstate/no_turbo"));
}

Result write_intel_pstate_no_turbo(const std::string &s) {
  assert(has_intel_pstate_no_turbo());
  std::string path("/sys/devices/system/cpu/intel_pstate/no_turbo");
  return write_str(path, s);
}

std::string read_intel_pstate_no_turbo() {
  assert(has_intel_pstate_no_turbo());
  std::string path("/sys/devices/system/cpu/intel_pstate/no_turbo");
  std::ifstream ifs(path, std::ifstream::in);
  std::string result;
  std::getline(ifs, result);
  return result;
}

bool is_turbo_enabled() { return "0" == read_intel_pstate_no_turbo(); }

Result disable_cpu_turbo() { return write_intel_pstate_no_turbo("1"); }
Result enable_cpu_turbo() { return write_intel_pstate_no_turbo("0"); }

} // namespace detail
} // namespace perfect