#pragma once

#include "perfect/result.hpp"

namespace perfect {
	namespace detail {

bool has_acpi_cpufreq_boost() {
  return bool(std::ifstream("/sys/devices/system/cpu/cpufreq/boost"));
}

int write_acpi_cpufreq_boost(const std::string &s) {
  assert(has_acpi_cpufreq_boost());
  std::string path("/sys/devices/system/cpu/cpufreq/boost");
  std::ofstream ofs(path, std::ofstream::out);
  ofs << s;
  ofs.close();
  if (ofs.fail()) {
    return 1;
  }
  return 0;
}

std::string read_acpi_cpufeq_boost() {
  assert(has_acpi_cpufreq_boost());
  std::string path("/sys/devices/system/cpu/cpufreq/boost");
  std::ifstream ifs(path, std::ifstream::in);
  std::string result;
  std::getline(ifs, result);
  return result;
}

    bool is_turbo_enabled() {
        return "1" == read_acpi_cpufeq_boost();
    }

    Result disable_cpu_turbo() {
        write_acpi_cpufreq_boost("0");
    }

    Result enable_cpu_turbo() {
        write_acpi_cpufreq_boost("1");
    }

} // namespace detail
} // namespace perfect
