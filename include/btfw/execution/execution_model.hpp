#pragma once

#include "btfw/core/market_state.hpp"
#include "btfw/execution/fill.hpp"
#include "btfw/execution/order.hpp"

namespace btfw {

class ExecutionModel {
 public:
  bool should_fill(const Order& order, const MarketState& market) const;
  Fill make_fill(const Order& order, const MarketState& market) const;
};

}  // namespace btfw
