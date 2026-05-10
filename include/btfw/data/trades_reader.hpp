#pragma once

#include "btfw/core/types.hpp"

#include <fstream>
#include <string>

namespace btfw {

class TradesReader {
 public:
  explicit TradesReader(const std::string& path);

  bool is_open() const;
  bool read_next(TradePrint& out);
  std::size_t skipped_rows() const;

 private:
  std::ifstream input_;
  std::string line_;
  std::size_t skipped_rows_{0};
};

}  // namespace btfw
