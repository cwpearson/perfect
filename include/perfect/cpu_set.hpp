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
#include <vector>

#include "init.hpp"
#include "result.hpp"

std::set<int> operator-(const std::set<int> &lhs, const std::set<int> &rhs) {
  std::set<int> result;
  for (auto e : lhs) {
    if (0 == rhs.count(e)) {
      result.insert(e);
    }
  }
  return result;
}

std::string remove_space(const std::string &s) {
  std::string result;

  for (auto c : s) {
    if (!isspace(c)) {
      result += c;
    }
  }
  return result;
}

// like "8" or "8-10"
std::set<int> parse_token(const std::string &token) {
  // std::cerr << "parse_token: parsing '" << s << "'\n";
  std::set<int> result;

  std::string s = token;
  // ignore empty string
  if (s.empty()) {
    return result;
  }

  // remove newline
  s = remove_space(s);

  size_t pos = 0;

  int first = std::stoi(s, &pos);
  // std::cerr << "parse_token: found '" << first << "'\n";

  // single int
  if (pos == s.length()) {
    result.insert(first);
    return result;
  }

  // next char should be a "-"
  assert(s[pos] == '-');

  std::string rest = s.substr(pos + 1);
  int second = std::stoi(rest, &pos);
  // std::cerr << "parse_token: found '" << second << "'\n";

  // insert first-second
  // std::cerr << "parse_token: range " << first << " to " << second << "\n";
  for (int i = first; i <= second; ++i) {
    result.insert(i);
  }
  return result;
}

std::set<int> parse_cpuset(const std::string &s) {
  // std::cerr << "parse_cpuset: parsing '" << s << "'\n";
  std::set<int> result;

  std::string token;
  std::stringstream ss(s);
  while (std::getline(ss, token, ',')) {

    if ("\n" != token) {
      auto newCpus = parse_token(token);
      for (auto cpu : newCpus) {
        result.insert(cpu);
      }
    }
  }

  return result;
}

// http://man7.org/linux/man-pages/man7/cpuset.7.html
namespace perfect {
class CpuSet {
public:
  std::string path_;
  std::set<int> cpus_;
  std::set<int> mems_;
  CpuSet *parent_;

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
        // std::cerr << "access error in mkdir: " << strerror(errno) << "\n";
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
          // std::cerr << "EBUSY in mount: " << strerror(errno) << "\n";
          return Result::SUCCESS;
        }
        case EPERM: {
        // std::cerr << "EPERM in mount: " << strerror(errno) << "\n";
        return Result::NO_PERMISSION;
        }
        case ENOENT:
        case EROFS:
        default:
          std::cerr << "unhandled error in mount: " << strerror(errno) << "\n";
          return Result::UNKNOWN;
        }
      }
    }
    return Result::SUCCESS;
  }

  std::string get_raw_cpus() {
    std::ifstream is(path_ + "/cpuset.cpus");
    std::stringstream ss;
    ss << is.rdbuf();
    return remove_space(ss.str());
  }

  std::string get_raw_mems() {
    std::ifstream is(path_ + "/cpuset.mems");
    std::stringstream ss;
    ss << is.rdbuf();
    return remove_space(ss.str());
  }

  std::set<int> get_cpus() { return parse_cpuset(get_raw_cpus()); }

  std::set<int> get_mems() { return parse_cpuset(get_raw_mems()); }

  // migrate the caller task from this cpu set to another
  Result migrate_self_to(CpuSet &other) {
    // enable memory migration in other
    other.enable_memory_migration();

    // get my pid
    pid_t self = this_task();

    // read this tasks and write each line to other.tasks
    std::ifstream is(path_ + "/tasks");
    std::string line;
    while (std::getline(is, line)) {
      line = remove_space(line);
      if (std::to_string(self) == line) {
        // std::cerr << "migrating self task " << line << " to " << other.path
        //           << "\n";
        other.write_task(line);
        return Result::SUCCESS;
      }
    }
    return Result::NO_TASK;
  }

  // migrate tasks in this cpu set to another
  Result migrate_tasks_to(CpuSet &other) {
    // enable memory migration in other
    PERFECT_SUCCESS_OR_RETURN(other.enable_memory_migration());

    // read this tasks and write each line to other.tasks
    std::ifstream is(path_ + "/tasks");
    std::string line;
    while (std::getline(is, line)) {
      // std::cerr << "migrating task " << line << " to " << other.path << "\n";
      other.write_task(line);
    }

    return Result::SUCCESS;
  }

  Result enable_memory_migration() {
    std::ofstream ofs(path_ + "/" + "cpuset.memory_migrate");
    ofs << "1";
    ofs.close();
    if (ofs.fail()) {
    switch (errno) {
    case EACCES:
      return Result::NO_PERMISSION;
    case ENOENT:
      return Result::NOT_SUPPORTED;
    default:
      return Result::UNKNOWN;
    }
  }
    return Result::SUCCESS;
  }

  void write_task(const std::string &task) {
    // write `task` to path/tasks
    std::ofstream os(path_ + "/tasks");
    os << task << "\n";
  }

  // object representing the root CPU set
  static Result get_root(CpuSet &root) {
    PERFECT_SUCCESS_OR_RETURN(CpuSet::init());
    root.path_ = "/dev/cpuset";
    root.parent_ = nullptr;
    return Result::SUCCESS;
  }

  // the ID of this task
  static pid_t this_task() { return getpid(); }

  Result make_child(CpuSet &child, const std::string &name) {

    if (mkdir((path_ + "/" + name).c_str(),
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

    child.path_ = path_ + "/" + name;
    child.parent_ = this;
    return Result::SUCCESS;
  }

  Result enable_cpu(const int cpu) {
    std::set<int> cpus = get_cpus();
    cpus.insert(cpu);
    return write_cpus(cpus);
  }

  Result enable_cpus(const std::set<int> &cpus) {
    std::set<int> finalCpus = get_cpus();
    for (auto cpu : cpus) {
      finalCpus.insert(cpu);
    }
    return write_cpus(finalCpus);
  }

  // FIXME: check error
  Result write_cpus(std::set<int> cpus) {
    std::ofstream os(path_ + "/cpuset.cpus");
    bool comma = false;
    for (auto cpu : cpus) {
      if (comma)
        os << ",";
      os << cpu << "-" << cpu;
      comma = true;
    }
    return Result::SUCCESS;
  }

  // FIXME: check write
  Result write_mems(std::set<int> mems) {
    std::ofstream os(path_ + "/cpuset.mems");
    bool comma = false;
    for (auto mem : mems) {
      if (comma)
        os << ",";
      os << mem << "-" << mem;
      comma = true;
    }
    return Result::SUCCESS;
  }

  Result enable_mem(const int mem) {
    std::set<int> mems = get_mems();
    mems.insert(mem);
    return write_mems(mems);
  }

  Result enable_mems(const std::set<int> &mems) {
    std::set<int> finalMems = get_mems();
    for (auto mem : mems) {
      finalMems.insert(mem);
    }
    return write_mems(finalMems);
  }

  Result destroy() {
    // remove all child cpu sets

    // move all attached processes back to parent
    assert(parent_);
    migrate_tasks_to(*parent_);

    // remove with rmdir
    if (rmdir(path_.c_str())) {
      switch (errno) {
      default:
        std::cerr << "unhandled error in rmdir: " << strerror(errno) << "\n";
        return Result::UNKNOWN;
      }
    }

    path_ = "";
    return Result::SUCCESS;
  }


};

  std::ostream &operator<<(std::ostream &s, const CpuSet &c) {
    s << c.path_;
    return s;
  }

} // namespace perfect
