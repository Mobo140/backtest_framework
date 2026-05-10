#pragma once

#include "btfw/core/types.hpp"

#include <cstdint>

namespace btfw {

using OrderId = std::uint64_t;

struct Order {
  OrderId id{};
  Side side{};
  Price price{};
  Quantity qty{};
};

}  // namespace btfw
