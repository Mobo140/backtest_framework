#include "btfw/execution/execution_model.hpp"

namespace btfw {

bool ExecutionModel::should_fill(const Order& order,
                                 const MarketState& market_state) const {
  if (order.side == Side::Buy) {
    return market_state.best_ask.price <= order.price;
  }

  if (order.side == Side::Sell) {
    return market_state.best_bid.price >= order.price;
  }

  return false;
}

Fill ExecutionModel::make_fill(const Order& order,
                               const MarketState& market_state) const {
  Fill fill;
  fill.order_id = order.id;
  fill.ts = market_state.ts;
  fill.side = order.side;
  fill.qty = order.qty;

  if (order.side == Side::Buy) {
    fill.price = market_state.best_ask.price;
  } else {
    fill.price = market_state.best_bid.price;
  }

  return fill;
}

}  // namespace btfw
