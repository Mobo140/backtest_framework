#pragma once

#include <cstdint>

namespace btfw {

using Timestamp = std::int64_t;
using Price = double;
using Quantity = double;

enum class Side {
  Buy,
  Sell
};

struct Level {
  Price price{};
  Quantity qty{};
};

struct TopOfBookSnapshot {
  Timestamp ts{};
  Level best_bid{};
  Level best_ask{};
};

struct TradePrint {
  Timestamp ts{};
  Side side{};
  Price price{};
  Quantity qty{};
};

}  // namespace btfw
