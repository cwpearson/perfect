#include <iostream>

#include <cmath>

#include <omp.h>
#include <cstring>
#include <chrono>
#include <vector>
#include <iostream>

int main(int argc, char **argv) {

size_t numThreads = std::stoi(argv[1]);
std::vector<size_t> totals(numThreads, 0);
omp_set_num_threads(numThreads);

auto start = std::chrono::system_clock::now();
double time = std::stod(argv[2]);

#pragma omp parallel
{
  size_t tid = omp_get_thread_num();
  double a = rand();
  while (true) {
    for (size_t i = 0; i < 500; ++i) {
      double x;
      asm volatile(""::"r"(a));
      x = sqrt(a);
      asm volatile(""::"r"(x));

      asm volatile(""::"r"(a));
      x = sqrt(a);
      asm volatile(""::"r"(x));
    }
    totals[tid] += 1000;
    auto elapsed = (std::chrono::system_clock::now() - start).count() / 1e9;
    if (elapsed > time) {
      break;
    }
  }
}

size_t sum = 0;
for (auto t : totals) {
  sum += t;
}
std::cout << (double)sum / time << "\n";

};
