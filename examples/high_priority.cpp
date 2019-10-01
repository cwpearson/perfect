#include <iostream>

#include "perfect/priority.hpp"

int main(void) {
  perfect::init();

  PERFECT(perfect::set_high_priority());

  // do things with high process scheduling priority

}