#include "btfw/strategy/simple_strategy.hpp"
#include <vector>

namespace btfw {

SimpleStrategy::SimpleStrategy(double target_position, double order_qty)
    : target_position_(target_position), order_qty_(order_qty) {}

  std::vector<Order> SimpleStrategy::on_market(const MarketState& market,
                                               double position) {
  if (market.best_ask.price <= 0.0 || market.best_bid.price <= 0.0) {
    return {};
  }

  if (order_qty_ <= 0.0) {
    return {};
  }

  if (position < target_position_) {
    return {{
      next_order_id_++,
      Side::Buy,
      market.best_ask.price,
      order_qty_
    }};
  }

  return {{
    next_order_id_++, 
    Side::Sell,
    market.best_bid.price, 
    order_qty_
  }};
}

}  // namespace btfw
