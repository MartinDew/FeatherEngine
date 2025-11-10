#include "math_defs.h"

#include <numbers>

namespace feather {

#ifdef DOUBLE_PRECISION
using real_t = double;
#else
using real_t = float;
#endif

real_t deg_to_rad(real_t degrees) {
	return degrees / 180.0f * std::numbers::pi;
}

real_t rad_to_deg(real_t radians) {
	return radians / std::numbers::pi * 180.0f;
}

Vector3 deg_to_rad(const Vector3& degrees) {
	Vector3 result;
	result.x = deg_to_rad(degrees.x);
	result.y = deg_to_rad(degrees.y);
	result.z = deg_to_rad(degrees.z);
	return result;
}

Vector3 rad_to_deg(const Vector3& degrees) {
	Vector3 result;
	result.x = rad_to_deg(degrees.x);
	result.y = rad_to_deg(degrees.y);
	result.z = rad_to_deg(degrees.z);
	return result;
}

uint32_t round_up_to_next_pow_2(uint32_t x) {
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	return x + 1;
}

Matrix convert_direction_vector_to_rotation_matrix(Vector3 forward) {
	Vector3 WorldUp = Vector3::unit_y;
	forward.normalize();

	// Make sure side vector is valid (direction could be close to WorldUp)
	Vector3 right = forward.cross(WorldUp);
	if (right.length_squared() < 0.001f) {
		Vector3 WorldRight = Vector3::right;
		right = forward.cross(WorldRight);
	}
	right.normalize();

	Vector3 up = forward.cross(right);
	return Matrix(forward, right, up);
}

constexpr uint32_t RaiseToNextMultipleOf(uint32_t val, uint32_t multiple) {
	if (val == 0)
		return 0;

	return val + (multiple - val % multiple);
}

} //namespace feather