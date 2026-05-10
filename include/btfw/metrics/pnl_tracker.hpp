#pragma once

#include "btfw/core/market_state.hpp"
#include "btfw/execution/fill.hpp"

namespace btfw {

class PnlTracker {
 public:
  void on_fill(const Fill& fill);
  void on_market(const MarketState& market_state);

  double cash() const;
  double position() const;
  double equity() const;
  double max_position() const;
  double min_position() const;
  double max_equity() const;
  double min_equity() const;

 private:
  double cash_{0.0};
  double position_{0.0};
  double equity_{0.0};
  double max_position_{0.0};
  double min_position_{0.0};
  double max_equity_{0.0};
  double min_equity_{0.0};
};

}  // namespace btfw
