#include <algorithm>
#include <iostream>

#include "perfect/cpu_set.hpp"

using namespace perfect;

int main(void) {

  perfect::init();

  CpuSet root;
  PERFECT(perfect::CpuSet::get_root(root));

  std::cerr << "cpus: " << root.get_raw_cpus() << "\n";
  std::cerr << "mems: " << root.get_raw_mems() << "\n";

  std::set<int> sCpus = {0};
  std::set<int> rCpus = root.get_cpus();
  std::set<int> uCpus = rCpus - sCpus;

  CpuSet s, u;

  PERFECT(root.make_child(s, "shielded"));
  PERFECT(root.make_child(u, "unshielded"));

  u.enable_memory_migration();

  // shield cpu 0
  s.enable_cpus(sCpus);
  u.enable_cpus(uCpus);

  u.enable_mem(0);
  s.enable_mem(0);

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