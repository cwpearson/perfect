#pragma once

namespace perfect {
namespace detail {

inline void flush_line(void *p) {

  /*
  PowerISA_V2.07B p. 773
  dcbf RA,RB,L

  effective address is RA|0 + RB
  this mnemonic has L=0, which is through all cache levels
  write block to storage and mark as invalid in all processors
  */

  /*!

   linux/arch/powerpc/include/asm/cache.h
  */
  asm volatile("dcbf 0, %0"
               : // no outputs
               : "r"(p)
               : "memory");


}

inline void barrier_all() {

  // sync is a mnemonic for sync 0, heavyweight sync
  asm volatile("sync"
               : // no outputs
               : // no inputs
               : "memory");


}

}
}
