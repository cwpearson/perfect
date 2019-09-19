#pragma once

#include <cassert>
#include <fstream>
#include <string>
#include <vector>

#include <sched.h>
#include <sys/types.h>
#include <unistd.h>

#include "perfect/result.hpp"

namespace perfect {

/*! return a set of CPUs the current thread can run on
 */
std::vector<int> cpus() {
  std::vector<int> result;
  cpu_set_t mask;
  if (sched_getaffinity(0 /*caller*/, sizeof(cpu_set_t), &mask)) {
    assert(0 && "failed sched_getaffinity");
  }
  for (int i = 0; i < CPU_SETSIZE; ++i) {
    if (CPU_ISSET(i, &mask)) {
      result.push_back(i);
    }
  }
  return result;
}

Result get_governor(std::string &result, const int cpu) {
  std::string path("/sys/devices/system/cpu/cpu");
  path += std::to_string(cpu);
  path += "/cpufreq/scaling_governor";
  std::ifstream ifs(path, std::ifstream::in);
  std::getline(ifs, result);
  return Result::SUCCESS;
}

Result set_governor(const int cpu, const std::string &governor) {
  std::string path("/sys/devices/system/cpu/cpu");
  path += std::to_string(cpu);
  path += "/cpufreq/scaling_governor";
  std::ofstream ofs(path, std::ofstream::out);
  ofs << governor;
  ofs.close();
  if (ofs.fail()) {
    return Result::NO_PERMISSION;
  }
  return Result::SUCCESS;
}

} // namespace perfect