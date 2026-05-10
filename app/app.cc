#include "btfw/app/application.hpp"
#include "btfw/data/lob_reader.hpp"
#include "btfw/data/trades_reader.hpp"
#include "btfw/execution/execution_model.hpp"
#include "btfw/replay/event_merger.hpp"
#include "btfw/replay/replay_engine.hpp"
#include "btfw/strategy/simple_strategy.hpp"
#include "btfw/strategy/as_strategy.hpp"

#include <fstream>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <string_view>

namespace btfw {

namespace {

std::uintmax_t file_size_or_zero(std::string_view path) {
  try {
    return std::filesystem::file_size(path);
  } catch (...) {
    return 0;
  }
}

void write_summary_txt(std::ostream& out,
                       const ReplayEngine& engine,
                       const MarketState& state,
                       const LobReader& lob_reader,
                       const TradesReader& trades_reader) {
  out << "Processed events: " << engine.processed_events() << '\n';
  out << "Processed LOB events: " << engine.processed_lob_events() << '\n';
  out << "Processed trade events: " << engine.processed_trade_events() << '\n';
  out << "Final market ts: " << state.ts << '\n';
  out << "Final best bid: " << state.best_bid.price
      << " qty=" << state.best_bid.qty << '\n';
  out << "Final best ask: " << state.best_ask.price
      << " qty=" << state.best_ask.qty << '\n';
  out << "Fills: " << engine.fills() << '\n';
  out << "Buy fills: " << engine.buy_fills() << '\n';
  out << "Sell fills: " << engine.sell_fills() << '\n';
  out << "Turnover: " << engine.turnover() << '\n';
  out << "Average fill price: " << engine.average_fill_price() << '\n';
  out << "Cash: " << engine.cash() << '\n';
  out << "Position: " << engine.position() << '\n';
  out << "Equity: " << engine.equity() << '\n';
  out << "Max position: " << engine.max_position() << '\n';
  out << "Min position: " << engine.min_position() << '\n';
  out << "Max equity: " << engine.max_equity() << '\n';
  out << "Min equity: " << engine.min_equity() << '\n';
  out << "LOB skipped rows: " << lob_reader.skipped_rows() << '\n';
  out << "Trade skipped rows: " << trades_reader.skipped_rows() << '\n';
}

void write_summary_csv(std::ostream& out,
                       const ReplayEngine& engine,
                       const MarketState& state,
                       const LobReader& lob_reader,
                       const TradesReader& trade_reader) {
  out << "processed_events,processed_lob_events,processed_trade_events,"
    << "fills,buy_fills,sell_fills,turnover,average_fill_price,"
    << "cash,position,equity,max_position,min_position,max_equity,min_equity,"
    << "final_ts,final_best_bid,final_best_bid_qty,final_best_ask,final_best_ask_qty,"
    << "lob_skipped_rows,trade_skipped_rows\n";

  out << engine.processed_events() << ','
      << engine.processed_lob_events() << ','
      << engine.processed_trade_events() << ','
      << engine.fills() << ','
      << engine.buy_fills() << ','
      << engine.sell_fills() << ','
      << engine.turnover() << ','
      << engine.average_fill_price() << ','
      << engine.cash() << ','
      << engine.position() << ','
      << engine.equity() << ','
      << engine.max_position() << ','
      << engine.min_position() << ','
      << engine.max_equity() << ','
      << engine.min_equity() << ','
      << state.ts << ','
      << state.best_bid.price << ','
      << state.best_bid.qty << ','
      << state.best_ask.price << ','
      << state.best_ask.qty << ','
      << lob_reader.skipped_rows() << ','
      << trade_reader.skipped_rows() << '\n';
}


void write_fills_csv(std::ostream& out, const ReplayEngine& engine) {
  out << "ts,side,price,qty,cash_after,position_after,equity_after\n";

  for (const auto& record : engine.fill_records()) {
    out << record.fill.ts << ','
        << (record.fill.side == Side::Buy ? "buy" : "sell") << ','
        << record.fill.price << ','
        << record.fill.qty << ','
        << record.cash_after << ','
        << record.position_after << ','
        << record.equity_after << '\n';
  }
}

void write_equity_curve_csv(std::ostream& out, const ReplayEngine& engine) {
  out << "ts,equity,position\n";
  for (const auto& snap : engine.equity_curve()) {
    out << snap.ts << ',' << snap.equity << ',' << snap.position << '\n';
  }
}


}  // namespace

int Application::run(const AppConfig& config) const {
  std::cout << "Backtest scaffold is ready.\n";
  std::cout << "LOB path: " << config.lob_path << '\n';
  std::cout << "Trades path: " << config.trades_path << '\n';

  if (config.max_events > 0) {
    std::cout << "Max events: " << config.max_events << '\n';
  } else {
    std::cout << "Max events: unlimited\n";
  }

  std::cout << "A-S params: gamma=" << config.as_gamma
            << " sigma=" << config.as_sigma
            << " kappa=" << config.as_kappa
            << " T=" << config.as_T
            << " max_pos=" << config.as_max_position << '\n';

  const auto lob_size = file_size_or_zero(config.lob_path);
  const auto trades_size = file_size_or_zero(config.trades_path);

  if (lob_size == 0 || trades_size == 0) {
    std::cerr << "Input files are missing or inaccessible.\n";
    return 1;
  }

  std::cout << "Input files found.\n";
  std::cout << "LOB bytes: " << lob_size << '\n';
  std::cout << "Trades bytes: " << trades_size << '\n';

  LobReader lob_reader(config.lob_path);
  if (!lob_reader.is_open()) {
    std::cerr << "Failed to open LOB file.\n";
    return 1;
  }

  TradesReader trades_reader(config.trades_path);
  if (!trades_reader.is_open()) {
    std::cerr << "Failed to open Trade file.\n";
    return 1;
  }

  EventMerger merger(lob_reader, trades_reader);

  AsParams as_params{
    .gamma          = config.as_gamma,
    .sigma          = config.as_sigma,
    .kappa          = config.as_kappa,
    .T              = config.as_T,
    .order_qty      = config.order_qty,
    .max_position   = config.as_max_position,
    .use_microprice  = config.as_use_microprice,
    .use_2018_spread = config.as_use_2018_spread,
  };
  AsStrategy strategy(as_params);

  ExecutionModel model;
  ReplayEngine engine(merger, strategy, model);

  engine.run(config.max_events);

  const auto& state = engine.market_state();

  write_summary_txt(std::cout, engine, state, lob_reader, trades_reader);

  std::filesystem::create_directories(config.report_dir);
  std::ofstream report(config.report_dir + "/summary.txt");

  if (!report.is_open()) {
    std::cerr << "Failed to open " << config.report_dir << "/summary.txt\n";
    return 1;
  }

  write_summary_txt(report, engine, state, lob_reader, trades_reader);

  std::ofstream summary_csv(config.report_dir + "/summary.csv");
  if (!summary_csv.is_open()) {
    std::cerr << "Failed to open " << config.report_dir << "/summary.csv\n";
    return 1;
  }

  write_summary_csv(summary_csv, engine, state, lob_reader, trades_reader);


  std::ofstream fills_csv(config.report_dir + "/fills.csv");
  if (!fills_csv.is_open()) {
    std::cerr << "Failed to open " << config.report_dir << "/fills.csv\n";
    return 1;
  }

  write_fills_csv(fills_csv, engine);

  std::ofstream equity_csv(config.report_dir + "/equity_curve.csv");
  if (!equity_csv.is_open()) {
    std::cerr << "Failed to open " << config.report_dir << "/equity_curve.csv\n";
    return 1;
  }

  write_equity_curve_csv(equity_csv, engine);

  return 0;
}

}  // namespace btfw
