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

add_executable(cpu-cache cpu_cache.cpp)
target_link_libraries(cpu-cache perfect)

add_executable(cpu-turbo cpu_turbo.cpp)
target_link_libraries(cpu-turbo perfect)

add_executable(os-perf os_perf.cpp)
target_link_libraries(os-perf perfect)

if(CMAKE_CUDA_COMPILER)
    add_executable(gpu-clocks gpu_clocks.cu)
    target_link_libraries(gpu-clocks perfect)
endif()

if(CMAKE_CUDA_COMPILER)
    add_executable(gpu-turbo gpu_turbo.cu)
    target_link_libraries(gpu-turbo perfect)
endif()

if(CMAKE_CUDA_COMPILER)
    add_executable(gpu-monitor gpu_monitor.cu)
    target_link_libraries(gpu-monitor perfect)
endif()