#pragma once

#include <cstddef>
#include <string>

namespace btfw {

struct AppConfig {
  std::string lob_path;
  std::string trades_path;

  // Maximum number of merged market events to replay. Zero means no limit.
  std::size_t max_events = 100;

  // Inventory target used by SimpleStrategy.
  double target_position = 20.0;

  // Quantity submitted in each strategy-generated order.
  double order_qty = 1.0;

  // Directory for summary and fill reports.
  std::string report_dir = "results";


  // Avellaneda-Stoikov parameters
  double as_gamma        = 0.1;
  double as_sigma        = 1e-5;   // price volatility per second for ~0.011 asset
  double as_kappa        = 1e7;    // arrival rate; keeps half_spread ≈ 1 tick
  double as_T            = 3600.0;
  double as_max_position = 400.0;

  bool as_use_microprice  = true;
  bool as_use_2018_spread = false;
};


}  // namespace btfw
