#include "btfw/replay/event_merger.hpp"

namespace btfw {

EventMerger::EventMerger(LobReader& lob_reader, TradesReader& trades_reader)
    : lob_reader_(lob_reader), trades_reader_(trades_reader) {
  has_next_lob_ = lob_reader_.read_next(next_lob_);
  has_next_trade_ = trades_reader_.read_next(next_trade_);
}

bool EventMerger::read_next(MarketEvent& out) {
  if (!has_next_lob_ && !has_next_trade_) {
    return false;
  }

  if (!has_next_trade_) {
    out.type = EventType::Lob;
    out.ts = next_lob_.ts;
    out.lob = next_lob_;
    has_next_lob_ = lob_reader_.read_next(next_lob_);

    return true;
  }

  if (!has_next_lob_) {
    out.type = EventType::Trade;
    out.ts = next_trade_.ts;
    out.trade = next_trade_;
    has_next_trade_ = trades_reader_.read_next(next_trade_);

    return true;
  }

  if (next_lob_.ts <= next_trade_.ts) {
    out.type = EventType::Lob;
    out.ts = next_lob_.ts;
    out.lob = next_lob_;
    has_next_lob_ = lob_reader_.read_next(next_lob_);

    return true;
  }

  out.type = EventType::Trade;
  out.ts = next_trade_.ts;
  out.trade = next_trade_;
  has_next_trade_ = trades_reader_.read_next(next_trade_);
  
  return true;
}

}  // namespace btfw

