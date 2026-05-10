#include "btfw/strategy/as_strategy.hpp"

#include <cmath>
#include <vector>

namespace btfw {

AsStrategy::AsStrategy(const AsParams& params) : params_(params) {}

std::vector<Order> AsStrategy::on_market(const MarketState& market, double position) {
    if (market.best_ask.price <= 0.0 || market.best_bid.price <= 0.0) {
        return {};
    }

    // Record that session has started
    if (first_ts_ < 0) {
        first_ts_ = market.ts;
    }

    const double t = static_cast<double>(market.ts - first_ts_) / 1e6;
    const double time_remaining = std::max(params_.T - t, 1e-6);

    const double total_qty = market.best_bid.qty + market.best_ask.qty;

    const double mid = (params_.use_microprice && total_qty > 0.0)
        ? (market.best_ask.price * market.best_bid.qty +
           market.best_bid.price * market.best_ask.qty) / total_qty
        : (market.best_bid.price + market.best_ask.price) / 2.0;


    // Reservation price
    const double r = mid - position * params_.gamma *
             params_.sigma * params_.sigma * time_remaining;    
        

    double half_spread;
    if (params_.use_2018_spread) {
        // A-S 2018
        half_spread = (1.0 / params_.kappa) *  std::log(1.0 + params_.kappa / params_.gamma)
            + (params_.gamma * params_.sigma * params_.sigma * time_remaining) / 2.0; 
    } else {
        // A-S 2008
        half_spread = 
        (params_.gamma * params_.sigma * params_.sigma * time_remaining) / 2.0
        + (1.0 / params_.gamma) * std::log(1.0 + params_.gamma / params_.kappa); 
    }
    // Optimal half-spread

    const double bid_price = r - half_spread;
    const double ask_price = r + half_spread;

    // Inventory guard
    if (std::abs(position) >= params_.max_position) {
        return {};
    }

    return {
        Order{next_order_id_++, Side::Buy, bid_price, params_.order_qty},
        Order{next_order_id_++, Side::Sell, ask_price, params_.order_qty},
    };
}

} // namespace btfw



