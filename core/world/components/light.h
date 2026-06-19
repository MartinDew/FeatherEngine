#pragma once

#include <math/math_defs.h>
#include <cstdint>

namespace feather {

struct Light {
	enum class Type : uint8_t {
		Directional,
		Point,
		Spot
	};

	Type type = Type::Directional;
	Vector3 position = Vector3::zero;
	Vector3 direction = Vector3::forward;
	Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
	float intensity = 1.0f;
	float range = 100.0f; // For point/spot lights
	float spot_angle = 45.0f; // For spot lights (in degrees)
	bool cast_shadows = true;
};

} //namespace feather