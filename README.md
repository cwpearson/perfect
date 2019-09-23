# perfect

| Branch | Status |
|-|-|
| master |[![Build Status](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Factions-badge.atrox.dev%2Fcwpearson%2Fperfect%2Fbadge%3Fref%3Dmaster&style=flat)](https://actions-badge.atrox.dev/cwpearson/perfect/goto?ref=master)|

CPU/GPU performance control library for benchmarking
* x86
* POWER
* Nvidia

## Features

- [x] GPU power/utilization/temperature monitoring (nvidia)
- [x] Disable CPU turbo (linux)
- [x] Set OS CPU performance mode to maximum (linux)
- [x] Set GPU clocks (nvidia)
- [x] Disable GPU turbo (nvidia)
- [x] Flush addresses from cache (amd64, POWER)
- [x] CUDA not required (GPU functions will not be compiled)
- [x] Flush file system caches (linux)

## Installing

### CMake 

Ensure you have CMake 3.13+.

Add the source tree to your project and then use add_subdirectory

```
git submodule add git@github.com:cwpearson/perfect.git thirdparty/perfect
```

`CMakeLists.txt`
```
...
add_subdirectory(thirdparty/perfect)
...
target_link_libraries(your-target perfect)
```

### Without CMake 
Download the source **AND**
* for compiling with a non-cuda compiler:
    * add the include directory to your includes
    * add `nvidia-ml` to your link flags
    * add `-DPERFECT_HAS_CUDA` to your compile definitions
* with a CUDA compiler, just compile normally (`PERFECT_HAS_CUDA` is defined for you)

```
g++ code_using_perfect.cpp -DPERFECT_HAS_CUDA -Iperfect/include -lnvidia-ml 
nvcc code_using_perfect.cu -Iperfect/include -lnvidia-ml
```

If you don't have CUDA, then you could just do
```
g++ code_using_perfect.cpp -I perfect/include
```

## Usage

The `perfect` functions all return a `perfect::Result`, which is defined in [include/perfect/result.hpp].
When things are working, it will be `perfect::Result::SUCCESS`.
A `PERFECT` macro is also defined, which will terminate with an error message unless the `perfect::Result` is `perfect::Result::SUCCESS`.

```c++
perfect::CpuTurboState state;
PERFECT(perfect::get_cpu_turbo_state(&state));
```

## Monitoring

`perfect` can monitor and record GPU activity.

See [examples/gpu_monitor.cu](examples/gpu_monitor.cu)

```c++
#include "perfect/gpu_monitor.hpp"
```

* `Monitor(std::ostream *stream)`: create a monitor that will write to `stream`.
* `void Monitor::start()`: start the monitor
* `void Monitor::stop()`: terminate the monitor
* `void Monitor::pause()`: pause the monitor thread
* `void Monitor::resume()`: resume the monitor thread

### Flush file system caches

`perfect` can drop various filesystem caches

See [tools/sync_drop_caches.cpp](tools/sync_drop_caches.cpp)

```c++
#include "perfect/drop_caches.hpp"
```

* `Result sync()`: flush filesystem caches to disk
* `Result drop_caches(DropCaches_t mode)`: remove file system caches
  * `mode = PAGECACHE`: drop page caches
  * `mode = ENTRIES`: drop dentries and inodes
  * `mode = PAGECACHE | ENTRIES`: both

### CPU Turbo

`perfect` can enable and disable CPU boost through the Intel p-state mechanism or the ACPI cpufreq mechanism.

See [examples/cpu_turbo.cpp](examples/cpu_turbo.cpp).


```c++
#include "perfect/cpu_turbo.hpp"
```

* `Result get_cpu_turbo_state(CpuTurboState *state)`: save the current CPU turbo state
* `Result set_cpu_turbo_state(CpuTurboState *state)`: restore a saved CPU turbo state
* `Result disable_cpu_turbo()`: disable CPU turbo
* `Result enable_cpu_turbo()`: enable CPU turbo
* `bool is_turbo_enabled(CpuTurboState state)`: check if turbo is enabled

### OS Performance

`perfect` can control the OS governor on linux.

See [examples/os_perf.cpp](examples/os_perf.cpp).

```c++
#include "perfect/os_perf.hpp"
```

* `Result get_os_perf_state(OsPerfState *state, const int cpu)`: Save the current OS governor mode for CPU `cpu`.
* `Result os_perf_state_maximum(const int cpu)`: Set the OS governor to it's maximum performance mode.
* `Result set_os_perf_state(const int cpu, OsPerfState state)`: Restore a previously-saved OS governor mode.

### GPU Turbo

`perfect` can enable/disable GPU turbo boost.

See [examples/gpu_turbo.cu](examples/gpu_turbo.cu).

```c++
#include "perfect/gpu_turbo.hpp"
```

* `Result get_gpu_turbo_state(GpuTurboState *state, unsigned int idx)`: Get the current turbo state for GPU `idx`, useful to restore later.
* `bool is_turbo_enabled(GpuTurboState state)`: Check if turbo is enabled.
* `Result set_gpu_turbo_state(GpuTurboState state, unsigned int idx)`: Set a previously saved turbo state.
* `Result disable_gpu_turbo(unsigned int idx)`: Disable GPU `idx` turbo.
* `Result enable_gpu_turbo(unsigned int idx)`: Enable GPU `idx` turbo.

### GPU Clocks

`perfect` can lock GPU clocks to their maximum values.

See [examples/gpu_clocks.cu](examples/gpu_clocks.cu).

```c++
#include "perfect/gpu_clocks.hpp"
```

* `Result set_max_gpu_clocks(unsigned int idx)`: Set GPU `idx` clocks to their maximum reported values.
* `Result reset_gpu_clocks(unsigned int idx)`: Unset GPU `idx` clocks.

### CPU Cache

`perfect` can flush data from CPU caches. Unlike the other APIs, these do not return a `Result` because they do not fail.

See [examples/cpu_cache.cpp](examples/cpu_cache.cpp).

```c++
#include "perfect/cpu_cache.hpp"
```

* `void flush_all(void *p, const size_t n)`: Flush all cache lines starting at `p` for `n` bytes.

## Changelog

* v0.2.0
    * add GPU monitoring
    * Make CUDA optional
* v0.1.0
    * cache control
    * Intel P-State control
    * linux governor control
    * POWER cpufreq control
    * Nvidia GPU boost control
    * Nvidia GPU clock control

## Wish List

- [ ] only monitor certain GPUs
- [ ] A wrapper utility
    - [ ] disable hyperthreading
    - [ ] reserve cores 
    - [ ] set process priority
    - [ ] disable ASLR