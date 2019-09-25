#pragma once

#include <fstream>
#include <string>

#include "perfect/result.hpp"

namespace perfect {
namespace detail {
Result write_str(const std::string &path, const std::string &val) {
  std::ofstream ofs(path);
  if (ofs.fail()) {
    return Result::NOT_SUPPORTED;
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