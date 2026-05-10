#pragma once

#include "btfw/core/market_state.hpp"
#include "btfw/execution/execution_model.hpp"
#include "btfw/metrics/pnl_tracker.hpp"
#include "btfw/replay/event_merger.hpp"
#include "btfw/strategy/strategy.hpp"
#include "btfw/replay/fill_record.hpp"

namespace btfw {

struct EquitySnapshot {
  Timestamp ts{};
  double equity{};
  double position{};
};

class ReplayEngine {
 public:
  explicit ReplayEngine(EventMerger& event_merger,
                        Strategy& strategy,
                        ExecutionModel& execution_model);

  const MarketState& market_state() const;
  const std::vector<EquitySnapshot>& equity_curve() const;
  std::size_t processed_events() const;
  std::size_t processed_lob_events() const;
  std::size_t processed_trade_events() const;
  std::size_t fills() const;
  std::size_t buy_fills() const;
  std::size_t sell_fills() const;

  void run(std::size_t max_events);

  double cash() const;
  double position() const;
  double equity() const;
  double turnover() const;
  double average_fill_price() const;
  double max_position() const;
  double min_position() const;
  double max_equity() const;
  double min_equity() const;
  const std::vector<FillRecord>& fill_records() const;

 private:
  EventMerger& merger_;
  Strategy& strategy_;
  ExecutionModel& execution_model_;

  MarketState market_state_;
  PnlTracker pnl_tracker_;

  std::size_t processed_events_{0};
  std::size_t processed_lob_events_{0};
  std::size_t processed_trade_events_{0};
  std::size_t fills_{0};
  std::size_t buy_fills_{0};
  std::size_t sell_fills_{0};
  std::vector<EquitySnapshot> equity_curve_;

  // Sum of fill price * qty across all executions.
  double turnover_{0.0};

  // Sum of filled quantity across all executions.
  double fill_qty_{0.0};

  std::vector<FillRecord> fill_records_;
  std::vector<Order> pending_orders_;
};

}  // namespace btfw
