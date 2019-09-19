#pragma once

#include <cassert>

namespace perfect {
    enum class Result {
        SUCCESS,
        NVIDIA_ML,
        NO_PERMISSION,
        UNKNOWN
    };

const char * get_string(const Result &result) {
    switch (result) {
        case Result::SUCCESS: return "success";
        case Result::NO_PERMISSION: return "no permission";
        case Result::UNKNOWN: return "unknown error";
        case Result::NVIDIA_ML: return "nvidia-ml error";
        default: assert(0 && "unexpected perfect::Result");
    }
}

}
