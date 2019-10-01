#pragma once

#ifdef __linux__
#include "detail/os/linux.hpp"
#else
#error "unsupported platform"
#endif

#include "init.hpp"

namespace perfect {
    Result set_high_priority() {
        return detail::set_high_priority();
    }
}