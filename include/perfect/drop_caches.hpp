#pragma once

#include <unistd.h>

#include <fstream>
#include <iostream>

#include "result.hpp"
#include "init.hpp"
#include "detail/fs.hpp"

namespace perfect {

enum DropCaches_t {
    PAGECACHE = 0x1,
    ENTRIES = 0x2
};


// commit filesystem caches to disk
Result sync() {
    // http://man7.org/linux/man-pages/man2/sync.2.html
    ::sync(); // always successful
    return Result::SUCCESS;
}

Result drop_caches(const DropCaches_t mode = DropCaches_t(PAGECACHE | ENTRIES)) {
    using detail::write_str;
    const std::string path = "/proc/sys/vm/drop_caches";
    if (mode & PAGECACHE & ENTRIES) {
        PERFECT_SUCCESS_OR_RETURN(write_str(path, "3"));
    } else if (mode & PAGECACHE) {
        PERFECT_SUCCESS_OR_RETURN(write_str(path, "1"));
    } else if (mode & ENTRIES) {
        PERFECT_SUCCESS_OR_RETURN(write_str(path, "2"));
    } else {
        std::cerr << "unexpected mode: " << mode << "\n";
        return Result::UNKNOWN;
    }
    return Result::SUCCESS;
}

}