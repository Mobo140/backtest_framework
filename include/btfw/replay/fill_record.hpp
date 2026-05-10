#pragma once

#include "btfw/execution/fill.hpp"

namespace btfw {

// Portfolio snapshot immediately after a fill is applied and marked to market.
struct FillRecord {
  Fill fill{};
  double cash_after{};
  double position_after{};
  double equity_after{};
};

}  // namespace btfw
