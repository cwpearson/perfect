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

add_executable(enable-turbo enable_turbo.cpp)
target_link_libraries(enable-turbo perfect)

