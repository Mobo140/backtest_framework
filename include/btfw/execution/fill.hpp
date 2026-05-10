#pragma once

#include "btfw/core/types.hpp"
#include "btfw/execution/order.hpp"

namespace btfw {

struct Fill {
  OrderId order_id{};
  Side side{};
  Price price{};
  Quantity qty{};
  Timestamp ts{};
};

}  // namespace btfw
