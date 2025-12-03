// A generic variant to ease out using specific types within the engine
#pragma once

#include <cstdint>
#include <variant>
#include <vector>

namespace feather {
using Variant = std::variant<std::monostate, int64_t, float, std::vector<uint8_t>>;
}