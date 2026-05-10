#pragma once

#include "btfw/core/config.hpp"

namespace btfw {

class Application {
public:
  int run(const AppConfig& config) const;
};

}  // namespace btfw
