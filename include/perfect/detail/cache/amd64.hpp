#pragma once

namespace perfect {
  namespace detail {



inline void flush_line(void *p) {
  /*!

  arch/x86/include/asm/special_insns.h

   p139
  https://www.amd.com/system/files/TechDocs/24594.pdf

  clflush mem8
  */

  asm volatile("clflush %0"
               : "+m"(p)
               : // no inputs
               : // no clobbers
  );
}

inline void barrier_all() {

  asm volatile("mfence"
               : // no outputs
               : // no inputs
               : "memory");
}

  }
}