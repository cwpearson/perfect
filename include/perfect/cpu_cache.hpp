#pragma once

/*!
Routines for controlling CPU caching
*/

#pragma once

#include <algorithm>
#include <cstdio>
#include <iostream>

#ifdef __linux__
#include "detail/os/linux.hpp"
#else
#error "unsupported OS"
#endif

#ifdef __powerpc__
#include "detail/cache/power.hpp"
#elif __amd64__
#include "detail/cache/amd64.hpp"
#else
#error "unsupported CPU arch"
#endif

#include "init.hpp"

namespace perfect {

inline void flush_all(void *p, const size_t n) {

  size_t lineSize = cache_linesize();

  // cache flush may not be ordered wrt other kinds of accesses
  detail::barrier_all();

  for (size_t i = 0; i < n; i += lineSize) {
    char *c = static_cast<char *>(p);
    detail::flush_line(&c[i]);
  }

  // make flushing visible to other accesses
  detail::barrier_all();
}

}