#pragma once

#ifdef __NVCC__
#ifndef PERFECT_HAS_CUDA
#define PERFECT_HAS_CUDA
#endif
#endif

#ifdef PERFECT_HAS_CUDA
#include <nvml.h>
#endif

#include <atomic>
#include <chrono>
#include <string>
#include <thread>

#include <iostream>

#include "perfect/init.hpp"

namespace perfect {

class Monitor {
public:
  struct Config {
    std::atomic<bool> stop;
    std::atomic<bool> pause;
    double samplePeriodMs;

    std::atomic<bool> power;
    std::atomic<bool> utilization;
    std::atomic<bool> temperature;
    std::atomic<bool> pstate;
    std::ostream *stream_;

    Config(std::ostream *stream)
        : stop(true), pause(false), power(true), samplePeriodMs(100),
          utilization(true), temperature(true), pstate(true), stream_(stream) {}
  };

  std::thread worker;
  Config config;

  Monitor(std::ostream *stream) : config(stream) {}

  static void worker_func(const Config &cfg) {

    nvmlReturn_t ret;
    nvmlDevice_t device;
    unsigned int deviceCount;

    nvmlUtilization_t utilization;
    unsigned int milliwatts;
    unsigned int temperature;
    nvmlPstates_t pState;

    ret = nvmlDeviceGetCount(&deviceCount);

    std::chrono::time_point<std::chrono::system_clock> start;

    while (!cfg.stop.load()) {
      if (std::chrono::time_point<std::chrono::system_clock>() == start) {
        start = std::chrono::system_clock::now();
      }
      if (!cfg.pause.load()) {

        const double elapsed =
            (std::chrono::system_clock::now() - start).count() / 1e9 * 1e3;

        for (unsigned int i = 0; i < deviceCount; ++i) {

          (*cfg.stream_) << elapsed << "," << i;

          ret = nvmlDeviceGetHandleByIndex(i, &device);

          if (cfg.power.load()) {
            ret = nvmlDeviceGetPowerUsage(device, &milliwatts);
            if (ret == NVML_SUCCESS) {
              (*cfg.stream_) << "," << milliwatts;
            } else {
              (*cfg.stream_) << "," << -1;
            }
          } else {
            (*cfg.stream_) << ","
                           << "x";
          }
          if (cfg.utilization.load()) {
            // period is between 1 second and 1/6 second depending on product
            ret = nvmlDeviceGetUtilizationRates(device, &utilization);
            if (ret == NVML_SUCCESS) {
              (*cfg.stream_)
                  << "," << utilization.gpu << "," << utilization.memory;
            } else {
              (*cfg.stream_) << "," << -1 << "," << -1;
            }
          } else {
            (*cfg.stream_) << ","
                           << "x"
                           << ","
                           << "x";
          }
          if (cfg.temperature.load()) {
            ret = nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU,
                                           &temperature);
            if (ret == NVML_SUCCESS) {
              (*cfg.stream_) << "," << temperature;
            } else {
              (*cfg.stream_) << "," << -1;
            }
          } else {
            (*cfg.stream_) << ","
                           << "x";
          }
          if (cfg.pstate.load()) {
            ret = nvmlDeviceGetPerformanceState(device, &pState);
            if (ret == NVML_SUCCESS) {
              (*cfg.stream_) << "," << pState;
            } else {
              (*cfg.stream_) << "," << -1;
            }
          } else {
            (*cfg.stream_) << ","
                           << "x";
          }

          (*cfg.stream_) << "\n";
        }
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }

  void start() {
    config.stop = false;
    worker = std::thread(worker_func, std::ref(config));
  }
  void stop() {
    config.stop = true;
    worker.join();
  }

  void resume() { config.pause.store(false); }
  void pause() { config.pause.store(true); }
};

} // namespace perfect