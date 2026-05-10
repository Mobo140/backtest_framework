#pragma once

#include "btfw/core/types.hpp"

namespace btfw {

enum class EventType {
  Lob,
  Trade
};

struct MarketEvent {
  EventType type{};
  Timestamp ts{};
  TopOfBookSnapshot lob{};
  TradePrint trade{};
};

}  // namespace btfw
