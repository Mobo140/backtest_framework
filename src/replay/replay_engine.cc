#include "btfw/replay/replay_engine.hpp"

#include <vector>

namespace btfw {

ReplayEngine::ReplayEngine(EventMerger& event_merger,
                           Strategy& strategy,
                           ExecutionModel& execution_model)
    : merger_(event_merger),
      strategy_(strategy),
      execution_model_(execution_model) {}

void ReplayEngine::run(std::size_t max_events) {
  MarketEvent event;
  std::size_t processed = 0;

  while ((max_events == 0 || processed < max_events) && merger_.read_next(event)) {
    if (event.type == EventType::Lob) {
      market_state_.ts = event.lob.ts;
      market_state_.best_ask = event.lob.best_ask;
      market_state_.best_bid = event.lob.best_bid;
      ++processed_lob_events_;

      // check orders from the PREVIOUS tick against the NEW market state.

      std::vector<Fill> tick_fills; // short time buffer (expired after 1 tick)
      for (const auto& order : pending_orders_) {
        if (!execution_model_.should_fill(order, market_state_)) continue;

        const Fill fill = execution_model_.make_fill(order, market_state_);
        pnl_tracker_.on_fill(fill);
        tick_fills.push_back(fill);

        if (fill.side == Side::Buy) ++buy_fills_;
        else if (fill.side == Side::Sell) ++sell_fills_;

        turnover_ += fill.price * fill.qty;
        fill_qty_ += fill.qty;
        ++fills_;
      }

      // strategy posts fresh quotes (uses updated position after fills).
      pending_orders_ = strategy_.on_market(market_state_, pnl_tracker_.position());

      // mark inventory to market.
      pnl_tracker_.on_market(market_state_);

      equity_curve_.push_back(EquitySnapshot{
        .ts = market_state_.ts,
        .equity = pnl_tracker_.equity(),
        .position = pnl_tracker_.position(),
      });

      // record all fills from this tick.
      for (const auto& fill : tick_fills) {
        fill_records_.push_back(FillRecord{
          .fill           = fill,
          .cash_after     = pnl_tracker_.cash(),
          .position_after = pnl_tracker_.position(),
          .equity_after   = pnl_tracker_.equity(),
        });
      }
    }

    if (event.type == EventType::Trade) {
      ++processed_trade_events_;
    }

    ++processed_events_;
    ++processed;
  }
}

const MarketState& ReplayEngine::market_state() const {
  return market_state_;
}

std::size_t ReplayEngine::processed_events() const {
  return processed_events_;
}

std::size_t ReplayEngine::processed_lob_events() const {
  return processed_lob_events_;
}

std::size_t ReplayEngine::processed_trade_events() const {
  return processed_trade_events_;
}

std::size_t ReplayEngine::fills() const {
  return fills_;
}

std::size_t ReplayEngine::buy_fills() const {
  return buy_fills_;
}

std::size_t ReplayEngine::sell_fills() const {
  return sell_fills_;
}

double ReplayEngine::cash() const {
  return pnl_tracker_.cash();
}

double ReplayEngine::position() const {
  return pnl_tracker_.position();
}

double ReplayEngine::equity() const {
  return pnl_tracker_.equity();
}

double ReplayEngine::turnover() const {
  return turnover_;
}

double ReplayEngine::max_position() const {
  return pnl_tracker_.max_position();
}

double ReplayEngine::min_position() const {
  return pnl_tracker_.min_position();
}

double ReplayEngine::max_equity() const {
  return pnl_tracker_.max_equity();
}

double ReplayEngine::min_equity() const {
  return pnl_tracker_.min_equity();
}

double ReplayEngine::average_fill_price() const {
  if (fill_qty_ == 0.0) {
    return 0.0;
  }

  return turnover_ / fill_qty_;
}

const std::vector<FillRecord>& ReplayEngine::fill_records() const {
  return fill_records_;
}

const std::vector<EquitySnapshot>& ReplayEngine::equity_curve() const {
  return equity_curve_;
}

}  // namespace btfw
