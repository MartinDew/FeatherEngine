#pragma once

#include "math/projection.h"
#include "math/transform.h"

namespace feather {

struct Viewport {
	struct Rect {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;

		bool is_auto() const noexcept { return width == 0 && height == 0; }
	};

	Rect rect;
	Transform camera_transform;
	Projection camera_projection;
};

} // namespace feather
