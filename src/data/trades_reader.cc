#include "btfw/data/trades_reader.hpp"

#include <sstream>
#include <string>
#include <vector>

namespace btfw {

TradesReader::TradesReader(const std::string& path) : input_(path) {
  if (input_.is_open()) {
    std::getline(input_, line_);
  }
}

bool TradesReader::is_open() const {
  return input_.is_open();
}

bool TradesReader::read_next(TradePrint& out) {
  while (std::getline(input_, line_)) {
    std::stringstream ss(line_);
    std::string field;
    std::vector<std::string> fields;

    while (std::getline(ss, field, ',')) {
      fields.push_back(field);
    }

    if (fields.size() < 5) {
      ++skipped_rows_;
      continue;
    }

    try {
      out.ts = std::stoll(fields[1]);

      if (fields[2] == "buy") {
        out.side = Side::Buy;
      } else if (fields[2] == "sell") {
        out.side = Side::Sell;
      } else {
        ++skipped_rows_;
        continue;
      }

      out.price = std::stod(fields[3]);
      out.qty = std::stod(fields[4]);

      return true;
    } catch (...) {
      ++skipped_rows_;
    }
  }

  return false;
}

std::size_t TradesReader::skipped_rows() const {
  return skipped_rows_;
}

}  // namespace btfw
