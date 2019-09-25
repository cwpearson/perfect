# removed -Wredundant-decls for cuda 10.1
# removed -Wundef for cuda 10.0
set(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} \
 -Xcompiler=-Wall\
 -Xcompiler=-Wextra\
 -Xcompiler=-Wcast-qual \
 -Xcompiler=-Wcast-align \
 -Xcompiler=-Wstrict-aliasing \
 -Xcompiler=-Wpointer-arith \
 -Xcompiler=-Winit-self \
 -Xcompiler=-Wshadow \
 -Xcompiler=-Wswitch-enum \
 -Xcompiler=-Wfloat-equal \
 -Xcompiler=-Wvla\
 -Xcompiler=-fmax-errors=1 \
 -Xcompiler=-Wfatal-errors\
 ")

 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
 -Wredundant-decls \
 -Wundef \
 -Wall\
 -Wextra\
 -Wcast-qual \
 -Wcast-align \
 -Wstrict-aliasing \
 -Wpointer-arith \
 -Winit-self \
 -Wshadow \
 -Wswitch-enum \
 -Wfloat-equal \
 -Wvla\
 -fmax-errors=1 \
 -Wfatal-errors\
 ")

add_executable(enable-cpu-turbo enable_cpu_turbo.cpp)
target_link_libraries(enable-cpu-turbo perfect)

add_executable(disable-cpu-turbo disable_cpu_turbo.cpp)
target_link_libraries(disable-cpu-turbo perfect)

add_executable(sync-drop-caches sync_drop_caches.cpp)
target_link_libraries(sync-drop-caches perfect)

add_executable(no-aslr no_aslr.cpp)
target_link_libraries(no-aslr perfect)

add_executable(max-os-perf max_os_perf.cpp)
target_link_libraries(max-os-perf perfect)

add_executable(min-os-perf min_os_perf.cpp)
target_link_libraries(min-os-perf perfect)

## OpenMP
find_package(OpenMP)
if (OpenMP_FOUND)
  add_executable(stress stress.cpp)
  target_link_libraries(stress perfect)
  target_link_libraries(stress OpenMP::OpenMP_CXX)
else(OpenMP_FOUND)
  message(WARNING "didn't find OpenMP, some benchmarks will be unavailable.")
endif(OpenMP_FOUND)

