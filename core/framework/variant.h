#pragma once

#include "high_level_array.h"
#include "reflected.h"

#include <any>
#include <cstdint>
#include <variant>
#include <vector>

namespace feather {

// A generic variant to ease out using specific types within the engine reflection. If an object, an any will be
// returned
using Variant = std::variant<std::monostate, int64_t, float, HighLevelArray, Reflected*>;

} //namespace feather