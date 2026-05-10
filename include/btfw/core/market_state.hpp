#pragma once

#include "btfw/core/types.hpp"

namespace btfw {

struct MarketState {
  Timestamp ts{};
  Level best_bid{};
  Level best_ask{};
};

}  // namespace btfw
