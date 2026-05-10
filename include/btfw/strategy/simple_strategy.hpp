#pragma once

#include "btfw/strategy/strategy.hpp"

namespace btfw {

class SimpleStrategy : public Strategy {
 public:
  SimpleStrategy(double target_position, double order_qty);

  std::vector<Order> on_market(const MarketState& market,
                                 double position) override;

 private:
  OrderId next_order_id_{1};
  double target_position_{0.0};
  double order_qty_{0.0};
};

}  // namespace btfw
