#pragma once

#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <set>
#include <sstream>
#include <string>

#include "init.hpp"
#include "result.hpp"

// like "8" or "8-10"
std::set<int> parse_token(const std::string &s) {
  std::set<int> result;
  size_t idx = 0;
  int first = std::stoi(s, &idx);

  // single int
  if (idx == s.length()) {
    result.insert(first);
    return result;
  }

  // next char should be a "-"
  assert(s[idx] == '-');

  idx += 1;
  int second = std::stoi(s, &idx);

  // insert first-second
  for (int i = first; i <= second; ++i) {
    result.insert(i);
  }
  return result;
}

std::set<int> parse_cpuset(const std::string &s) {
  std::string delim = ",";
  std::set<int> result;

  auto begin = 0u;
  auto end = s.find(delim);
  while (end != std::string::npos) {
    std::string token = s.substr(begin, end - begin);
    auto newCpus = parse_token(token);
    for (auto cpu : newCpus) {
      result.insert(cpu);
    }

    begin = end + delim.length();
    end = s.find(delim, begin);
  }

  return result;
}

// http://man7.org/linux/man-pages/man7/cpuset.7.html
namespace perfect {
class CpuSet {
public:
  std::string path;
  std::set<int> cpus;
  std::set<int> mems;
  CpuSet *parent;

  // make sure cpuset is initialized
  static Result init() {

    // check for "nodev cpuset" in /proc/filesystems

    // mkdir /dev/cpuset
    if (mkdir("/dev/cpuset", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) {
      switch (errno) {
      case EEXIST: {
        // okay
        break;
      }
      case EACCES:
        return Result::NO_PERMISSION;
      case ENOENT:
      case EROFS:
      default:
        std::cerr << "unhandled error in mkdir: " << strerror(errno) << "\n";
        return Result::UNKNOWN;
      }

      // mount -t cpuset none /dev/cpuset
      if (mount("none", "/dev/cpuset", "cpuset", 0, nullptr)) {
        switch (errno) {
        case EEXIST: {
          // okay
          break;
        }
        case EBUSY: {
          // FIXME: something is mounted here, assume it is what we want
          return Result::SUCCESS;
        }
        case EACCES:
          return Result::NO_PERMISSION;
        case ENOENT:
        case EROFS:
        default:
          std::cerr << "unhandled error in mount: " << strerror(errno) << "\n";
          return Result::UNKNOWN;
        }
      }
    }
  }

  std::set<int> get_cpus() {
    std::ifstream is(path + "/cpuset.cpus");
    std::stringstream ss;
    ss << is.rdbuf();
    return parse_cpuset(ss.str());
  }

  std::set<int> get_mems() {
    std::ifstream is(path + "/cpuset.mems");
    std::stringstream ss;
    ss << is.rdbuf();
    return parse_cpuset(ss.str());
  }

  // migrate tasks in this cpu set to another
  void migrate_tasks_to(CpuSet &other) {
    // enable memory migration in other
    other.enable_memory_migration();

    // read this tasks and write each line to other.tasks
    std::ifstream is(path + "/tasks");
    std::string line;
    while (std::getline(is, line)) {
      std::cerr << "migrating task " << line << " to " << other.path << "\n";
      other.write_task(line);
    }
  }

  void enable_memory_migration() {
    std::cerr << "enable memory migration in " << path << "\n";
    std::ofstream os(path + "/" + "cpuset.memory_migrate");
    os << "1";
  }

  void write_task(const std::string &task) {
    // write `task` to path/tasks
    std::ofstream os(path + "/tasks");
    os << task << "\n";
  }

  // object representing the root CPU set
  static Result get_root(CpuSet &root) {
    Result result;
    result = CpuSet::init();
    if (Result::SUCCESS != result) {
      return result;
    }
    root.path = "/dev/cpuset";
    root.parent = nullptr;
    return Result::SUCCESS;
  }

  Result make_child(CpuSet &child, const std::string &name) {

    if (mkdir((path + "/" + name).c_str(),
              S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) {
      switch (errno) {
      case EEXIST: {
        // okay
        break;
      }
      case EACCES:
        return Result::NO_PERMISSION;
      case ENOENT:
      case EROFS:
      default:
        return Result::UNKNOWN;
      }
    }

    child.path = path + "/" + name;
    child.parent = this;
    return Result::SUCCESS;
  }

  Result enable_cpu(const int cpu) {
    std::set<int> cpus = get_cpus();
    cpus.insert(cpu);
    write_cpus(cpus);
  }

  Result write_cpus(std::set<int> cpus) {
    std::ofstream os(path + "/" + "cpuset.cpus");
    bool comma = false;
    for (auto cpu : cpus) {
      if (comma)
        os << ",";
      os << cpu << "-" << cpu + 1;
      comma = true;
    }
  }

  Result write_mems(std::set<int> mems) {
    std::ofstream os(path + "/" + "cpuset.mems");
    bool comma = false;
    for (auto mem : mems) {
      if (comma)
        os << ",";
      os << mem << "-" << mem + 1;
      comma = true;
    }
  }

  Result enable_mem(const int mem) {
    std::set<int> mems = get_mems();
    mems.insert(mem);
    write_mems(mems);
  }

  Result destroy() {
    // remove all child cpu sets

    // move all attached processes back to parent
    assert(parent);
    migrate_tasks_to(*parent);

    // remove with rmdir
    if (rmdir(path.c_str())) {
      switch (errno) {
      default:
        std::cerr << "unhandled error in rmdir: " << strerror(errno) << "\n";
        return Result::UNKNOWN;
      }
    }

    path = "";
    return Result::SUCCESS;
  }
};

} // namespace perfect
