#pragma once

#include "btfw/core/types.hpp"

#include <fstream>
#include <string>

namespace btfw {

class LobReader {
 public:
  explicit LobReader(const std::string& path);

  bool is_open() const;
  bool read_next(TopOfBookSnapshot& out);
  std::size_t skipped_rows() const;

 private:
  std::ifstream input_;
  std::string line_;
  std::size_t skipped_rows_{0};
};

}  // namespace btfw
