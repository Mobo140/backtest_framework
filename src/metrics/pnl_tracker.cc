#include "btfw/metrics/pnl_tracker.hpp"

#include <algorithm>

namespace btfw {

void PnlTracker::on_fill(const Fill& fill) {
  if (fill.side == Side::Buy) {
    position_ += fill.qty;
    cash_ -= fill.price * fill.qty;
    max_position_ = std::max(max_position_, position_);
    min_position_ = std::min(min_position_, position_);
    return;
  }

  if (fill.side == Side::Sell) {
    position_ -= fill.qty;
    cash_ += fill.price * fill.qty;
    max_position_ = std::max(max_position_, position_);
    min_position_ = std::min(min_position_, position_);
  }
}

void PnlTracker::on_market(const MarketState& market_state) {
  // Mark the current inventory to the midpoint of the top of book.
  const double mid =
      (market_state.best_bid.price + market_state.best_ask.price) / 2.0;
  equity_ = cash_ + position_ * mid;
  max_equity_ = std::max(max_equity_, equity_);
  min_equity_ = std::min(min_equity_, equity_);
}

double PnlTracker::cash() const {
  return cash_;
}

double PnlTracker::position() const {
  return position_;
}

double PnlTracker::equity() const {
  return equity_;
}

double PnlTracker::max_position() const {
  return max_position_;
}

double PnlTracker::min_position() const {
  return min_position_;
}

double PnlTracker::max_equity() const {
  return max_equity_;
}

double PnlTracker::min_equity() const {
  return min_equity_;
}

}  // namespace btfw
