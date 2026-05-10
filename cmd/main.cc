#include "btfw/app/application.hpp"

#include <cstdlib>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace {

void print_usage() {
  std::cerr
      << "Usage: bt_runner --lob <path> --trades <path> [--max-events <n>] "
      << "[--order-qty <x>] [--report-dir <path>] "
      << "[--as-gamma <x>] [--as-sigma <x>] [--as-kappa <x>] "
      << "[--as-T <x>] [--as-max-position <x>] "
      << "[--as-microprice] [--as-2018-spread]\n";
}

std::optional<std::size_t> parse_size(std::string_view value) {
  try {
    std::size_t consumed = 0;
    const auto parsed = std::stoull(std::string(value), &consumed);
    if (consumed != value.size()) {
      return std::nullopt;
    }
    return parsed;
  } catch (...) {
    return std::nullopt;
  }
}

std::optional<double> parse_double(std::string_view value) {
  try {
    std::size_t consumed = 0;
    const auto parsed = std::stod(std::string(value), &consumed);
    if (consumed != value.size()) {
      return std::nullopt;
    }
    return parsed;
  } catch (...) {
    return std::nullopt;
  }
}

}  // namespace

int main(int argc, char** argv) {
  btfw::AppConfig config;

  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];

    if (arg == "--lob" && i + 1 < argc) {
      config.lob_path = argv[++i];
      continue;
    }

    if (arg == "--trades" && i + 1 < argc) {
      config.trades_path = argv[++i];
      continue;
    }

    if (arg == "--max-events" && i + 1 < argc) {
      const auto parsed = parse_size(argv[++i]);
      if (!parsed.has_value()) {
        std::cerr << "Invalid value for --max-events\n";
        return 1;
      }
      config.max_events = *parsed;
      continue;
    }

    if (arg == "--target-position" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) {
        std::cerr << "Invalid value for --target-position\n";
        return 1;
      }
      config.target_position = *parsed;
      continue;
    }

    if (arg == "--order-qty" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) {
        std::cerr << "Invalid value for --order-qty\n";
        return 1;
      }
      config.order_qty = *parsed;
      continue;
    }

    if (arg == "--report-dir" && i + 1 < argc) {
      config.report_dir = argv[++i];
      continue;
    }

    if (arg == "--as-gamma" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) { std::cerr << "Invalid value for --as-gamma\n"; return 1; }
      config.as_gamma = *parsed;
      continue;
    }

    if (arg == "--as-sigma" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) { std::cerr << "Invalid value for --as-sigma\n"; return 1; }
      config.as_sigma = *parsed;
      continue;
    }

    if (arg == "--as-kappa" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) { std::cerr << "Invalid value for --as-sigma\n"; return 1; }
      config.as_kappa = *parsed;
      continue;
    }

    if (arg == "--as-T" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) { std::cerr << "Invalid value for --as-T\n"; return 1; }
      config.as_T = *parsed;
      continue;
    }

    if (arg == "--as-max-position" && i + 1 < argc) {
      const auto parsed = parse_double(argv[++i]);
      if (!parsed.has_value()) { std::cerr << "Invalid value for --as-max-position\n"; return 1; }
      config.as_max_position = *parsed;
      continue;
    }

    if (arg == "--as-microprice") {
      config.as_use_microprice = true;
      continue;
    }

    if (arg == "--no-as-microprice") {
      config.as_use_microprice = false;
      continue;
    }

    if (arg == "--as-2018-spread") {
      config.as_use_2018_spread = true;
      continue;
    }

    if (arg == "--help" || arg == "-h") {
      print_usage();
      return 0;
    }

    std::cerr << "Unknown or incomplete argument: " << arg << '\n';
    print_usage();
    return 1;
  }

  if (config.lob_path.empty() || config.trades_path.empty()) {
    print_usage();
    return 1;
  }

  const btfw::Application app;
  return app.run(config);
}
