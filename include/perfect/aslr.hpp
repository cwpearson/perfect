#pragma once

#include <cerrno>
#include <iostream>

#ifdef __linux__
#include "detail/os/linux.hpp"
#endif
#include "init.hpp"
#include "result.hpp"

namespace perfect {

struct AslrState {
#ifdef __linux__
  unsigned long persona;
#else
#error "unsupported platform"
#endif
};

Result get_aslr(AslrState &state) {
  int persona;
  PERFECT_SUCCESS_OR_RETURN(detail::get_personality(persona));
  state.persona = persona;
  return Result::SUCCESS;
}

Result set_aslr(const AslrState &state) {
  return detail::set_personality(state.persona);
}

Result disable_aslr() {
  int persona;
  PERFECT_SUCCESS_OR_RETURN(detail::get_personality(persona));
  persona |= ADDR_NO_RANDOMIZE;
  return detail::set_personality(persona);
}

} // namespace perfect