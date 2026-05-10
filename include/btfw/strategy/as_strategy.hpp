#pragma once 


#include "btfw/strategy/strategy.hpp"

namespace btfw {

struct AsParams {
    double gamma = 0.1; // risk aversion
    double sigma = 1.0; // volatility (per second)
    double kappa = 1.5; // order arrival rate
    double T = 3600.0; // session time
    double order_qty = 1.0; // size of each posted order 
    double max_position = 10.0; // inventory limit 
    bool use_microprice = true; // use microprice instead of naive mid
    bool use_2018_spread = false;
};


class AsStrategy  : public Strategy {
    public: 
        explicit AsStrategy(const AsParams& params);
        std::vector<Order> on_market(const MarketState& market, double position) override;
    
    private:
        AsParams params_;
        OrderId next_order_id_{1};
        Timestamp first_ts_{-1}; // timestamp of the first tick 
    
};   

} // namespace btfw
