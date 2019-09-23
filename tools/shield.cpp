#include <iostream>

#include "perfect/cpu_set.hpp"

using namespace perfect;

int main(void) {

  perfect::init();

  Result ret;
  CpuSet root;
  PERFECT(perfect::CpuSet::get_root(root));

  CpuSet s, u;

  PERFECT(root.make_child(s, "shielded"));
  PERFECT(root.make_child(u, "unshielded"));

  u.enable_memory_migration();
  u.enable_cpu(0);
  u.enable_mem(0);

  root.migrate_tasks_to(u);

  s.destroy();
  u.destroy();
}