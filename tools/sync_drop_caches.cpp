#include <iostream>

#include "perfect/drop_caches.hpp"

using namespace perfect;

int main(void) {

  using namespace perfect;

  PERFECT(init());
  PERFECT(perfect::sync());
  PERFECT(drop_caches(DropCaches_t(PAGECACHE | ENTRIES)));
}