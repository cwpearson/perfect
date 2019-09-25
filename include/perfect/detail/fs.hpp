#pragma once

#include <fstream>
#include <string>

#include "../result.hpp"

#ifdef __linux__
#include "fs/linux.hpp"
#else
#error "unsupported platform"
#endif

namespace perfect {
namespace detail {

Result write_str(const std::string &path, const std::string &val) {

  if (!path_exists(path)) {
    std::cerr << "write_str(): does not exist: " << path << "\n";
    return Result::NOT_SUPPORTED;
  }

  std::ofstream ofs(path);
  if (ofs.fail()) {
    std::cerr << "failed to open " << path << "\n";
    return Result::NO_PERMISSION;
  }

  ofs << val;
  ofs.close();
  if (ofs.fail()) {
    switch (errno) {
    case EACCES:
    std::cerr << "EACCES when writing to " << path << "\n";
      return Result::NO_PERMISSION;
    case EPERM:
      std::cerr << "EPERM when writing to " << path << "\n";
      return Result::NO_PERMISSION;
    case ENOENT:
    std::cerr << "ENOENT when writing to " << path << "\n";
      return Result::NOT_SUPPORTED;
    default:
      return Result::UNKNOWN;
    }
  }
  return Result::SUCCESS;
}

} // namespace detail
} // namespace perfect