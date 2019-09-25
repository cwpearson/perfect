# 3.8+ for project(LANGUAGES CUDA)
# 3.9+ for OpenMP::OpenMP_CXX
# 3.10+ findopenmp gained support for language-specific components
# 3.11+ for CMake not to add -fopenmp to the nvcc flags
# 3.13+ for target_link_directories
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(perfect LANGUAGES CXX VERSION 0.5.0)
message(STATUS "Build type: " ${CMAKE_BUILD_TYPE})

include(CheckLanguage)

option(USE_CUDA "try to use CUDA" ON)

if(USE_CUDA)
  check_language(CUDA)
  if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
  else()
    message(STATUS "No CUDA support")
  endif()
endif()

#https://blog.kitware.com/cmake-and-the-default-build-type/
# Set a default build type if none was specified
set(default_build_type "Release")
if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  set(default_build_type "Debug")
endif()
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


if (CMAKE_BUILD_TYPE MATCHES Debug)
  message(STATUS "Setting verbose build during Debug")
  set(CMAKE_VERBOSE_MAKEFILE ON)
  set(CMAKE_CUDA_FLAGS ${CMAKE_CUDA_FLAGS} -G)
elseif (CMAKE_BUILD_TYPE MATCHES Release)
  add_definitions(-DNDEBUG)
  set(CMAKE_CUDA_FLAGS ${CMAKE_CUDA_FLAGS} -lineinfo)
endif()

add_subdirectory(include/perfect)

add_library(perfect INTERFACE)
target_compile_features(perfect INTERFACE cxx_std_11)
if(CMAKE_CUDA_COMPILER)
  target_compile_definitions(perfect INTERFACE -DPERFECT_HAS_CUDA)
  # tell the host compiler where to find the CUDA includes and libraries
  # CMakeFiles/<version>/CMakeCUDACompiler.cmake
  target_include_directories(perfect INTERFACE ${CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES})
  target_link_directories(perfect INTERFACE ${CMAKE_CUDA_HOST_IMPLICIT_LINK_DIRECTORIES})
  target_link_libraries(perfect INTERFACE nvidia-ml)
endif()
target_include_directories(perfect INTERFACE include/)


add_subdirectory(examples)
add_subdirectory(tools)