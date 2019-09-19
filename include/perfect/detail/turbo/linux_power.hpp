#pragma once

#include "perfect/result.hpp"

namespace perfect {

bool has_acpi_cpufreq_boost() {
  return bool(std::ifstream("/sys/devices/system/cpu/cpufreq/boost"));
}

int write_acpi_cpufreq_boost(const std::string &s) {
  assert(has_acpi_cpufreq_boost());
  std::string path("/sys/devices/system/cpu/cpufreq/boost");
  SPDLOG_LOGGER_TRACE(logger::console(), "writing to {}", path);
  std::ofstream ofs(path, std::ofstream::out);
  ofs << s;
  ofs.close();
  if (ofs.fail()) {
    SPDLOG_LOGGER_TRACE(logger::console(), "error writing to {}", path);
    return 1;
  }
  return 0;
}

std::string read_acpi_cpufeq_boost() {
  assert(has_acpi_cpufreq_boost());
  std::string path("/sys/devices/system/cpu/cpufreq/boost");
  SPDLOG_LOGGER_TRACE(logger::console(), "reading {}", path);
  std::ifstream ifs(path, std::ifstream::in);
  std::string result;
  std::getline(ifs, result);
  return result;
}

    bool is_turbo_enabled() {
        return "1" == read_acpi_cpufeq_boost();
    }

    Result disable_cpu_turbo() {
        write_acpi_cpufeq_boost("0");
    }

    Result enable_cpu_turbo() {
        write_acpi_cpufeq_boost("1");
    }

}