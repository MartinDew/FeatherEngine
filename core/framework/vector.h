#pragma once

#include <memory>
#include <vector>

namespace feather {
template <class T, class TAlloc = std::allocator<T>>
using vector = std::vector<T, TAlloc>;
}