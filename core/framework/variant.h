#pragma once

#include <any>
#include <cstdint>
#include <variant>
#include <vector>

namespace feather {

class HighLevelArray;

// A generic variant to ease out using specific types within the engine reflection. If an object, an any will be
// returned
using Variant = std::variant<std::monostate, int64_t, float, HighLevelArray, std::any>;

} //namespace feather