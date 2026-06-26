#pragma once
#include <cstdint>

namespace feather {

enum class Notification : std::uint32_t {
	NONE = 0,
	WINDOW_SHOWN,
	WINDOW_RESIZED,
	COUNT
};

} //namespace feather