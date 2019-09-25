#pragma once

#include <string>
#include <cerrno>
#include <iostream>
#include <cassert>



#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace perfect {
    namespace detail {
        bool path_exists(const std::string &path) {
            struct stat sb;
            if (stat(path.c_str(), &sb)) {
                switch (errno) {
                    case ENOENT: return false;
                    case ENOTDIR: return false;
                    default: {
                        std::cerr << "unhandled error in stat() for " << path << "\n";
                        assert(0);
                    }
                }
            }
            return true;
        }
    }
}