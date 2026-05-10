#pragma once

#include <vector>

#include "btfw/core/market_state.hpp"
#include "btfw/execution/order.hpp"

namespace btfw {

class Strategy {
 public:
  virtual ~Strategy() = default;

  virtual std::vector<Order> on_market(const MarketState& market,
                                         double position) = 0;
};

}  // namespace btfw
