# perfect

| Branch | Status |
|-|-|
| master |[![Build Status](https://img.shields.io/endpoint.svg?url=https%3A%2F%2Factions-badge.atrox.dev%2Fcwpearson%2Fperfect%2Fbadge%3Fref%3Dmaster&style=flat)](https://actions-badge.atrox.dev/cwpearson/perfect/goto?ref=master) |

CPU/GPU Performance control library for benchmarking on Linux, x86, POWER, and Nvidia.

## Features

- [x] GPU power/utilization/temperature monitoring (nvidia)
- [x] Disable CPU turbo (linux)
- [x] Set OS CPU performance mode to maximum (linux)
- [x] Set GPU clocks (nvidia)
- [x] Disable GPU turbo (nvidia)
- [x] Flush addresses from cache (amd64, POWER)
- [x] CUDA not required (GPU functions will not be compiled)
- [x] Flush file system caches (linux)
- [x] Disable ASLR (linux)

## Contributors
* [Carl Pearson](https://cwpearson.github.io)

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

## Tools Usage

### tools/perfect-cli

`perfect` provides some useful tools on Linux:

```
$ tools/perfect-cli -h
SYNOPSIS
        ./tools/perfect-cli --no-mod [-n <INT>] -- <cmd>...
        ./tools/perfect-cli ([-u <INT>] | [-s <INT>]) [--no-drop-cache] [--no-max-perf] [--aslr]
                            [--cpu-turbo] [--stdout <PATH>] [--stderr <PATH>] [-n <INT>] -- <cmd>...

OPTIONS
        --no-mod               don't control performance
        -u                     number of unshielded CPUs
        -s                     number of shielded CPUs
        --no-drop-cache        do not drop filesystem caches
        --no-max-perf          do not max os perf
        --aslr                 enable ASLR
        --cpu-turbo            enable CPU turbo
        --stdout               redirect child stdout
        --stderr               redirect child stderr
        -n                     run multiple times
```

The basic usage is `tools/perfect-cli -- my-exe`, which will attempt to configure the system for repeatable performance before executing `my-exe`, and then restore the system to the original performance state before exiting.
Most modifications require elevated privileges.
The default behavior is to:
* disable ASLR
* set CPU performance to maximum
* disable CPU turbo
* drop filesystem caches before each iteration

Some options (all should provided before the `--` option):
* `--no-mod` flag will cause `perfect-cli` to not modify the system performance state
* `-n INT` will run the requested program `INT` times.
* `--stderr`/`--stdout` will redirect the program-under-test's stderr and stdout to the provided paths.
* `-s`/`-u`: set the number of shielded /unshielded CPUs. The program-under-test will run on the shielded CPUs. All other tasks will run on the unshielded CPUs.

A common invocation might look like:
```
sudo tools/perfect-cli -n 5 --stderr=run.err --stdout=run.out -- ./my-benchmark
```
This will disable ASLR, set CPU performance to maximum, disable CPU turbo, and then run `./my-benchmark` 5 times after dropping the filesystem cache before each run, redirecting stdout/stderr of ./my-benchmark to `run.out`/`run.err`.
The owner of `run.out` and `run.err` will be set to whichever user called `sudo`.

### tools/addr

Print the address of `main`, a stack variable, and a heap variable.
Useful for demoing ASLR.

### tools/no-aslr

Disable ASLR on the provided execution.

With ASLR, addresses are different with each invocation
```
$ tools/addr
main:  94685074364704
stack: 140734279743492
heap:  94685084978800
$ tools/addr
main:  93891046344992
stack: 140722671706708
heap:  93891068624496
```

Without ASLR, addresses are the same in each invocation
```
$ tools/no-aslr tools/addrs       
main:  93824992233760
stack: 140737488347460
heap:  93824994414192
$ tools/no-aslr tools/addrs       
main:  93824992233760
stack: 140737488347460
heap:  93824994414192
```

## API Usage

The `perfect` functions all return a `perfect::Result`, which is defined in [include/perfect/result.hpp].
When things are working, it will be `perfect::Result::SUCCESS`.
A `PERFECT` macro is also defined, which will terminate with an error message unless the `perfect::Result` is `perfect::Result::SUCCESS`.

```c++
perfect::CpuTurboState state;
PERFECT(perfect::get_cpu_turbo_state(&state));
```

### Monitoring

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

### Disable ASLR

`perfect` can disable ASLR

See [tools/no_aslr.cpp](tools/no_aslr.cpp)

```c++
#include "perfect/aslr.hpp"
```

* `Result disable_aslr()`: disable ASLR
* `Result get_aslr(AslrState &state)`: save the current ASLR state
* `Result set_aslr(const AslrState &state)`: set a previously-saved ASLR state


### Flush file system caches

`perfect` can drop various filesystem caches

See [tools/sync_drop_caches.cpp](tools/sync_drop_caches.cpp)

```c++
#include "perfect/drop_caches.hpp"
```

* `Result sync()`: flush filesystem caches to disk
* `Result drop_caches(DropCaches_t mode = DropCaches_t(PAGECACHE | ENTRIES))`: remove file system caches
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

* `Result get_os_perf_state(OsPerfState &state)`: Save the current OS governor mode for all CPUs.
* `Result os_perf_state_maximum(const int cpu)`: Set the OS governor to it's maximum performance mode.
* `Result set_os_perf_state(OsPerfState state)`: Restore a previously-saved OS governor mode.

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

* v0.5.0
    * add tools/stress
    * add tools/max-os-perf
    * add tools/min-os-perf
    * add tools/enable-cpu-turbo
    * add tools/disable-cpu-turbo
* v0.4.0
    * Add ASLR interface
    * Disambiguate some filesystem errors
    * Fix some powerpc namespace issues
* v0.3.0
    * Add filesystem cache interface
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
- [ ] hyperthreading interface
- [ ] process priority interface
- [ ] A wrapper utility
    - [ ] disable hyperthreading
    - [ ] reserve cores 
    - [ ] set process priority
    - [ ] disable ASLR

## Related

* [LLVM benchmarking instructions](https://llvm.org/docs/Benchmarking.html#linux) covering ASLR, Linux governor, cpuset shielding, SMT, and Intel turbo.
* [easyperf.net](https://easyperf.net/blog/2019/08/02/Perf-measurement-environment-on-Linux#2-disable-hyper-threading) blog post discussing ACPI/Intel turbo, SMT, Linux governor, CPU affinity, process priority, file system caches, and ASLR. 
* [temci](https://github.com/parttimenerd/temci) benchmarking tool for cpu sheilding and disabling hyperthreading, among other things.
* [perflock](https://github.com/aclements/perflock) tool for locking CPU frequency scaling domains

## Acks

Uses [muellan/clipp](https://github.com/muellan/clipp) for cli option parsing.
Uses [martinmoene/optional-lite](https://github.com/martinmoene/optional-lite).
