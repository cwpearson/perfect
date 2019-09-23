#include <algorithm>
#include <iostream>

#include "perfect/cpu_set.hpp"

using namespace perfect;

int main(void) {

  perfect::init();

  CpuSet root;
  PERFECT(perfect::CpuSet::get_root(root));

  std::cerr << "root cpus: " << root.get_raw_cpus() << "\n";
  std::cerr << "root mems: " << root.get_raw_mems() << "\n";

  // shield CPU 0
  std::set<int> sCpus = {0};
  std::set<int> rCpus = root.get_cpus();
  std::set<int> uCpus = rCpus - sCpus;

  CpuSet s, u;

  PERFECT(root.make_child(s, "shielded"));
  PERFECT(root.make_child(u, "unshielded"));

  // enable specific cpus for the children
  s.enable_cpus(sCpus);
  u.enable_cpus(uCpus);

  // enable all memories for the children
  u.enable_mems(root.get_mems());
  s.enable_mems(root.get_mems());

  // migrate the caller to s
  std::cerr << "migrate self to " << s << "\n";
  PERFECT(root.migrate_self_to(s));

  std::cerr << "migrate others to " << u << "\n";
  PERFECT(root.migrate_tasks_to(u));

  std::cerr << "clean up " << s << "\n";
  PERFECT(s.destroy());
  std::cerr << "clean up " << u << "\n";
  PERFECT(u.destroy());
}