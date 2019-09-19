#pragma once

#include <cassert>
#include <fstream>

#include "perfect/result.hpp"

namespace perfect {
namespace detail {

bool has_intel_pstate_no_turbo() {
  return bool(std::ifstream("/sys/devices/system/cpu/intel_pstate/no_turbo"));
}

Result write_intel_pstate_no_turbo(const std::string &s) {
  assert(has_intel_pstate_no_turbo());
  std::string path("/sys/devices/system/cpu/intel_pstate/no_turbo");
  std::ofstream ofs(path, std::ofstream::out);
  ofs << s;
  ofs.close();
  if (ofs.fail()) {
    return Result::NO_PERMISSION;
  }
  return Result::SUCCESS;
}

std::string read_intel_pstate_no_turbo() {
  assert(has_intel_pstate_no_turbo());
  std::string path("/sys/devices/system/cpu/intel_pstate/no_turbo");
  //   SPDLOG_LOGGER_TRACE(logger::console(), "reading {}", path);
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