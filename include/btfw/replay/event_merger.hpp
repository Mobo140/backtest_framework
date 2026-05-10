#pragma once

#include "btfw/core/event.hpp"
#include "btfw/data/lob_reader.hpp"
#include "btfw/data/trades_reader.hpp"

namespace btfw {

class EventMerger {
 public:
  EventMerger(LobReader& lob_reader, TradesReader& trades_reader);

  bool read_next(MarketEvent& out);

 private:
  LobReader& lob_reader_;
  TradesReader& trades_reader_;

  bool has_next_lob_{false};
  bool has_next_trade_{false};

  TopOfBookSnapshot next_lob_{};
  TradePrint next_trade_{};
};

}  // namespace btfw