#pragma once

#include <unistd.h>

#include <set>
#include <string>

// http://man7.org/linux/man-pages/man7/cpuset.7.html

class CpuSet {

  std::string path;
  std::set<int> cpus;
  std::set<int> mems;

  // migrate tasks in this cpu set to another
  void migrate_to(CpuSet &other) {
    // enable memory migration in other

    // read this tasks and write each line to other.tasks
  }

  void write_task(const std::string &task) {
    // write `task` to path/tasks
  }

  // object representing the root CPU set
  static CpuSet get_root() {

    CpuSet cpuset;
    cpuset.path = "/dev/cpuset";

    // mount /dev/cpuset
  }

  CpuSet make_child(const std::string &name) {
    // path is path/name
  }

  void destroy() {
    // remove all child cpu sets
    // remove all attached processes
    // remove with rmdir
  }
};

#if 0

auto root = CpuSet::get_root();

auto allCpus = root.cpus;

shielded = std::set<int>{0};
auto unshielded = std::set_difference(allCpus, shielded)

CpuSet s = root.make_child("shielded");
CpuSet u = root.make_child("unshielded");

root.migrate_to(u);

// put self in s

// exec child

// join child

// move everyone back to root
u.migrate_to(root);
s.migrate_to(root);

// destroy children, which should be empty at this point
s.destroy();
u.destroy();

#endif