#include "btfw/data/lob_reader.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace btfw {

LobReader::LobReader(const std::string& path) : input_(path) {
  if (input_.is_open()) {
    std::getline(input_, line_);
  }
}

bool LobReader::is_open() const {
  return input_.is_open();
}

bool LobReader::read_next(TopOfBookSnapshot& out) {
  while (std::getline(input_, line_)) {
    std::stringstream ss(line_);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(ss, field, ',')) {
      fields.push_back(field);
    }

    if (fields.size() < 6) {
      ++skipped_rows_;
      continue;
    }

    try {
      out.ts = std::stoll(fields[1]);
      out.best_ask.price = std::stod(fields[2]);
      out.best_ask.qty = std::stod(fields[3]);
      out.best_bid.price = std::stod(fields[4]);
      out.best_bid.qty = std::stod(fields[5]);

      return true;
    } catch (...) {
      ++skipped_rows_;
    }
  }

  return false;
}

std::size_t LobReader::skipped_rows() const {
  return skipped_rows_;
}

}  // namespace btfw
