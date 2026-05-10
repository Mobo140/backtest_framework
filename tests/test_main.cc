#include "btfw/core/market_state.hpp"
#include "btfw/data/lob_reader.hpp"
#include "btfw/data/trades_reader.hpp"
#include "btfw/execution/execution_model.hpp"
#include "btfw/execution/fill.hpp"
#include "btfw/execution/order.hpp"
#include "btfw/metrics/pnl_tracker.hpp"
#include "btfw/replay/event_merger.hpp"
#include "btfw/replay/replay_engine.hpp"
#include "btfw/strategy/as_strategy.hpp"
#include "btfw/strategy/simple_strategy.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string_view>

namespace {

bool approx_equal(double lhs, double rhs, double eps = 1e-9) {
  return std::fabs(lhs - rhs) <= eps;
}

bool check(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << "FAIL: " << message << '\n';
    return false;
  }
  return true;
}

bool test_execution_model_buy_fill() {
  btfw::ExecutionModel model;

  const btfw::MarketState market{
      .ts = 100,
      .best_bid = {99.0, 10.0},
      .best_ask = {100.0, 12.0},
  };

  const btfw::Order order{
      .id = 1,
      .side = btfw::Side::Buy,
      .price = 100.0,
      .qty = 2.0,
  };

  if (!check(model.should_fill(order, market), "buy order should fill at best ask")) {
    return false;
  }

  const btfw::Fill fill = model.make_fill(order, market);
  return check(fill.order_id == 1, "buy fill order id mismatch") &&
         check(fill.side == btfw::Side::Buy, "buy fill side mismatch") &&
         check(fill.ts == 100, "buy fill timestamp mismatch") &&
         check(approx_equal(fill.price, 100.0), "buy fill price mismatch") &&
         check(approx_equal(fill.qty, 2.0), "buy fill qty mismatch");
}

bool test_execution_model_sell_fill() {
  btfw::ExecutionModel model;

  const btfw::MarketState market{
      .ts = 200,
      .best_bid = {101.0, 8.0},
      .best_ask = {102.0, 9.0},
  };

  const btfw::Order order{
      .id = 2,
      .side = btfw::Side::Sell,
      .price = 101.0,
      .qty = 3.0,
  };

  if (!check(model.should_fill(order, market), "sell order should fill at best bid")) {
    return false;
  }

  const btfw::Fill fill = model.make_fill(order, market);
  return check(fill.order_id == 2, "sell fill order id mismatch") &&
         check(fill.side == btfw::Side::Sell, "sell fill side mismatch") &&
         check(fill.ts == 200, "sell fill timestamp mismatch") &&
         check(approx_equal(fill.price, 101.0), "sell fill price mismatch") &&
         check(approx_equal(fill.qty, 3.0), "sell fill qty mismatch");
}

bool test_execution_model_no_fill() {
  btfw::ExecutionModel model;

  const btfw::MarketState market{
      .ts = 300,
      .best_bid = {99.0, 5.0},
      .best_ask = {100.0, 5.0},
  };

  const btfw::Order order{
      .id = 3,
      .side = btfw::Side::Buy,
      .price = 99.5,
      .qty = 1.0,
  };

  return check(!model.should_fill(order, market), "buy order should not fill below best ask");
}

bool test_pnl_tracker() {
  btfw::PnlTracker tracker;

  tracker.on_fill(btfw::Fill{
      .order_id = 1,
      .side = btfw::Side::Buy,
      .price = 10.0,
      .qty = 2.0,
      .ts = 1,
  });

  tracker.on_fill(btfw::Fill{
      .order_id = 2,
      .side = btfw::Side::Sell,
      .price = 11.0,
      .qty = 1.0,
      .ts = 2,
  });

  tracker.on_market(btfw::MarketState{
      .ts = 3,
      .best_bid = {11.0, 10.0},
      .best_ask = {13.0, 10.0},
  });

  return check(approx_equal(tracker.cash(), -9.0), "cash mismatch") &&
         check(approx_equal(tracker.position(), 1.0), "position mismatch") &&
         check(approx_equal(tracker.equity(), 3.0), "equity mismatch");
}

bool test_simple_strategy() {
  btfw::SimpleStrategy strategy(2.0, 0.5);

  const btfw::MarketState market{
      .ts = 1,
      .best_bid = {99.0, 10.0},
      .best_ask = {100.0, 10.0},
  };

  const auto first_vec = strategy.on_market(market, 0.0);
  if (!check(!first_vec.empty(), "simple strategy should create buy order")) {
    return false;
  }

  const auto& first = first_vec[0];

  const auto second_vec = strategy.on_market(market, 3.0);
  if (!check(!second_vec.empty(), "simple strategy should create sell order above target position")) {
    return false;
  }

  const auto& second = second_vec[0];

  const auto third_vec = strategy.on_market(market, 2.0);
  if (!check(!third_vec.empty(), "simple strategy should create sell order at target position")) {
    return false;
  }

  const auto& third = third_vec[0];

  return check(first.side == btfw::Side::Buy, "first simple strategy order should be buy") &&
         check(approx_equal(first.price, 100.0), "buy order should use best ask") &&
         check(approx_equal(first.qty, 0.5), "buy order qty mismatch") &&
         check(second.side == btfw::Side::Sell, "second simple strategy order should be sell") &&
         check(approx_equal(second.price, 99.0), "sell order should use best bid") &&
         check(approx_equal(second.qty, 0.5), "sell order qty mismatch") &&
         check(third.side == btfw::Side::Sell,
               "third simple strategy order should be sell at target position") &&
         check(approx_equal(third.price, 99.0),
               "sell-at-target order should use best bid") &&
         check(approx_equal(third.qty, 0.5),
               "sell-at-target order qty mismatch") &&
         check(second.id == first.id + 1, "order id should increment") &&
         check(third.id == second.id + 1, "order id should increment across sells");
}

bool test_event_merger_orders_events_by_timestamp() {
  const std::filesystem::path temp_dir =
      std::filesystem::temp_directory_path() / "btfw_event_merger_test";
  std::filesystem::create_directories(temp_dir);

  const std::filesystem::path lob_path = temp_dir / "lob.csv";
  const std::filesystem::path trades_path = temp_dir / "trades.csv";

  {
    std::ofstream lob_file(lob_path);
    lob_file
        << ",local_timestamp,asks[0].price,asks[0].amount,bids[0].price,bids[0].amount\n"
        << "0,200,101.0,10,100.0,9\n"
        << "1,400,103.0,8,102.0,7\n";
  }

  {
    std::ofstream trades_file(trades_path);
    trades_file
        << ",local_timestamp,side,price,amount\n"
        << "0,100,buy,100.5,1\n"
        << "1,300,sell,101.5,2\n";
  }

  btfw::LobReader lob_reader(lob_path.string());
  btfw::TradesReader trades_reader(trades_path.string());
  if (!check(lob_reader.is_open(), "event merger test failed to open lob csv")) {
    return false;
  }
  if (!check(trades_reader.is_open(), "event merger test failed to open trades csv")) {
    return false;
  }

  btfw::EventMerger merger(lob_reader, trades_reader);
  btfw::MarketEvent event;

  if (!check(merger.read_next(event), "event merger failed on first event")) {
    return false;
  }
  if (!check(event.type == btfw::EventType::Trade, "first merged event should be trade")) {
    return false;
  }
  if (!check(event.ts == 100, "first merged event timestamp mismatch")) {
    return false;
  }

  if (!check(merger.read_next(event), "event merger failed on second event")) {
    return false;
  }
  if (!check(event.type == btfw::EventType::Lob, "second merged event should be lob")) {
    return false;
  }
  if (!check(event.ts == 200, "second merged event timestamp mismatch")) {
    return false;
  }

  if (!check(merger.read_next(event), "event merger failed on third event")) {
    return false;
  }
  if (!check(event.type == btfw::EventType::Trade, "third merged event should be trade")) {
    return false;
  }
  if (!check(event.ts == 300, "third merged event timestamp mismatch")) {
    return false;
  }

  if (!check(merger.read_next(event), "event merger failed on fourth event")) {
    return false;
  }
  if (!check(event.type == btfw::EventType::Lob, "fourth merged event should be lob")) {
    return false;
  }
  if (!check(event.ts == 400, "fourth merged event timestamp mismatch")) {
    return false;
  }

  return check(!merger.read_next(event), "event merger should be exhausted after four events");
}

bool test_lob_reader_skips_malformed_rows() {
  const std::filesystem::path temp_dir =
      std::filesystem::temp_directory_path() / "btfw_lob_reader_test";
  std::filesystem::create_directories(temp_dir);

  const std::filesystem::path lob_path = temp_dir / "lob.csv";

  {
    std::ofstream lob_file(lob_path);
    lob_file
        << ",local_timestamp,asks[0].price,asks[0].amount,bids[0].price,bids[0].amount\n"
        << "0,bad_ts,101.0,10,100.0,9\n"
        << "1,200,102.0,11,101.0,8\n";
  }

  btfw::LobReader reader(lob_path.string());
  if (!check(reader.is_open(), "lob reader test failed to open csv")) {
    return false;
  }

  btfw::TopOfBookSnapshot snapshot;
  if (!check(reader.read_next(snapshot), "lob reader failed to read valid row after bad row")) {
    return false;
  }

  return check(snapshot.ts == 200, "lob reader timestamp mismatch") &&
         check(approx_equal(snapshot.best_ask.price, 102.0), "lob reader ask price mismatch") &&
         check(approx_equal(snapshot.best_ask.qty, 11.0), "lob reader ask qty mismatch") &&
         check(approx_equal(snapshot.best_bid.price, 101.0), "lob reader bid price mismatch") &&
         check(approx_equal(snapshot.best_bid.qty, 8.0), "lob reader bid qty mismatch") &&
         check(reader.skipped_rows() == 1, "lob reader skipped row count mismatch");
}

bool test_trades_reader_skips_malformed_rows() {
  const std::filesystem::path temp_dir =
      std::filesystem::temp_directory_path() / "btfw_trades_reader_test";
  std::filesystem::create_directories(temp_dir);

  const std::filesystem::path trades_path = temp_dir / "trades.csv";

  {
    std::ofstream trades_file(trades_path);
    trades_file
        << ",local_timestamp,side,price,amount\n"
        << "0,100,broken_side,100.5,1\n"
        << "1,200,buy,101.5,2\n";
  }

  btfw::TradesReader reader(trades_path.string());
  if (!check(reader.is_open(), "trades reader test failed to open csv")) {
    return false;
  }

  btfw::TradePrint trade;
  if (!check(reader.read_next(trade),
             "trades reader failed to read valid row after bad row")) {
    return false;
  }

  return check(trade.ts == 200, "trades reader timestamp mismatch") &&
         check(trade.side == btfw::Side::Buy, "trades reader side mismatch") &&
         check(approx_equal(trade.price, 101.5), "trades reader price mismatch") &&
         check(approx_equal(trade.qty, 2.0), "trades reader qty mismatch") &&
         check(reader.skipped_rows() == 1, "trades reader skipped row count mismatch");
}

bool test_replay_engine_tracks_fill_records() {
  const std::filesystem::path temp_dir =
      std::filesystem::temp_directory_path() / "btfw_replay_engine_test";
  std::filesystem::create_directories(temp_dir);

  const std::filesystem::path lob_path = temp_dir / "lob.csv";
  const std::filesystem::path trades_path = temp_dir / "trades.csv";

  {
    std::ofstream lob_file(lob_path);
    lob_file
        << ",local_timestamp,asks[0].price,asks[0].amount,bids[0].price,bids[0].amount\n"
        << "0,100,101.0,10,100.0,9\n"
        << "1,200,101.0,10,100.0,9\n"  // same ask → buy@101 from tick 0 fills here
        << "2,300,103.0,10,102.0,9\n";
  }

  {
    std::ofstream trades_file(trades_path);
    trades_file
        << ",local_timestamp,side,price,amount\n";
  }

  btfw::LobReader lob_reader(lob_path.string());
  btfw::TradesReader trades_reader(trades_path.string());
  if (!check(lob_reader.is_open(), "replay engine test failed to open lob csv")) {
    return false;
  }
  if (!check(trades_reader.is_open(), "replay engine test failed to open trades csv")) {
    return false;
  }

  btfw::EventMerger merger(lob_reader, trades_reader);
  btfw::SimpleStrategy strategy(2.0, 1.0);
  btfw::ExecutionModel model;
  btfw::ReplayEngine engine(merger, strategy, model);

  engine.run(0);

  return check(engine.fills() > 0, "replay engine should produce at least one fill") &&
         check(engine.fill_records().size() == engine.fills(),
               "fill record count should match fill count");
}


bool test_microprice_pulled_toward_pressure_side() {
  // bid_qty=100 >> ask_qty=10 → buying pressure → microprice closer to ask
  const btfw::MarketState market{
      .ts = 0,
      .best_bid = {99.0, 100.0},
      .best_ask = {101.0, 10.0},
  };

  btfw::AsParams p;
  p.use_microprice = true;
  btfw::AsStrategy strategy(p);

  const auto orders = strategy.on_market(market, 0.0);
  if (!check(orders.size() == 2, "microprice strategy should return 2 orders")) {
    return false;
  }

  // reservation price = microprice - 0 (position=0), quoted mid = (bid+ask)/2
  const double quoted_mid = (orders[0].price + orders[1].price) / 2.0;
  const double naive_mid = (99.0 + 101.0) / 2.0;  // 100.0

  // microprice = (101*100 + 99*10) / 110 = (10100+990)/110 = 11090/110 ≈ 100.818
  // so quoted_mid should be above naive_mid
  return check(quoted_mid > naive_mid,
               "quoted mid should be above naive mid when bid-heavy (buying pressure)");
}

bool test_2018_spread_differs_from_2008() {
  const btfw::MarketState market{
      .ts = 0,
      .best_bid = {99.0, 10.0},
      .best_ask = {101.0, 10.0},
  };

  btfw::AsParams p2008;
  p2008.use_2018_spread = false;
  p2008.use_microprice  = false;

  btfw::AsParams p2018;
  p2018.use_2018_spread = true;
  p2018.use_microprice  = false;

  btfw::AsStrategy s2008(p2008), s2018(p2018);
  const auto o08 = s2008.on_market(market, 0.0);
  const auto o18 = s2018.on_market(market, 0.0);

  if (!check(o08.size() == 2, "2008 strategy should return 2 orders")) return false;
  if (!check(o18.size() == 2, "2018 strategy should return 2 orders")) return false;

  const double spread08 = o08[1].price - o08[0].price;
  const double spread18 = o18[1].price - o18[0].price;

  return check(std::abs(spread08 - spread18) > 1e-9,
               "2018 spread should differ from 2008 spread");
}

}  // namespace

int main() {
  const bool ok =
      test_execution_model_buy_fill() &&
      test_execution_model_sell_fill() &&
      test_execution_model_no_fill() &&
      test_pnl_tracker() &&
      test_simple_strategy() &&
      test_event_merger_orders_events_by_timestamp() &&
      test_lob_reader_skips_malformed_rows() &&
      test_trades_reader_skips_malformed_rows() &&
      test_replay_engine_tracks_fill_records() &&
      test_microprice_pulled_toward_pressure_side() &&
      test_2018_spread_differs_from_2008();

  if (!ok) {
    return 1;
  }

  std::cout << "All tests passed.\n";
  return 0;
}
