#include <chrono>
#include <iostream>
#include <thread>

#include "perfect/gpu_monitor.hpp"

int main(void) {
  using namespace perfect;
  init();

  // write to stderr
  Monitor m(&std::cerr);

  // don't record GPU utilization
  m.config.utilization = false;

  m.start();

  // ctrl-c to exit
  while (true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
  }

  return 0;
}