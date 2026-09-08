#pragma once
// The engine's math vocabulary: Feather's own types, computed through Realtime
// Math (see rtm_interop.h). The unsuffixed names below follow the precision the
// build selected -- math/precision.h.
#include "precision.h"

#include <array>
#include <cmath>
#include <format>
#include <type_traits>
#include <utility>

namespace feather {

using UByteColor = std::array<uint8_t, 4>;

// Plain integer vectors: no arithmetic yet, they only carry values through the
// engine (mesh indices, viewport extents). TODO: give them the operator set.
struct Vector2ui { uint32_t x = 0, y = 0; };
struct Vector3ui { uint32_t x = 0, y = 0, z = 0; };
struct Vector4ui { uint32_t x = 0, y = 0, z = 0, w = 0; };

struct Vector2i { int32_t x = 0, y = 0; };
struct Vector3i { int32_t x = 0, y = 0, z = 0; };
struct Vector4i { int32_t x = 0, y = 0, z = 0, w = 0; };

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector2 uv;

	Vertex() = default;
	constexpr Vertex(Vector3 pos, Vector3 normal, Vector2 uv = Vector2::zero) : position(pos), normal(normal), uv(uv) {}
	constexpr Vertex(real_t px, real_t py, real_t pz, real_t nx, real_t ny, real_t nz, real_t u = 0, real_t v = 0)
			: position(px, py, pz)
			, normal(nx, ny, nz)
			, uv(u, v) {}
	Vertex(const Vertex&) = default;
	Vertex& operator=(const Vertex&) = default;
	Vertex(Vertex&&) = default;
	Vertex& operator=(Vertex&&) = default;

	bool operator==(const Vertex&) const;
};

struct AABB {
	Vector3 min;
	Vector3 max;

	AABB() = default;
	constexpr AABB(Vector3 min, Vector3 max) : min(min), max(max) {}
	constexpr AABB(real_t min_x, real_t min_y, real_t min_z, real_t max_x, real_t max_y, real_t max_z)
			: min(min_x, min_y, min_z)
			, max(max_x, max_y, max_z) {}
	AABB(const AABB&) = default;
	AABB& operator=(const AABB&) = default;
	AABB(AABB&&) = default;
	AABB& operator=(AABB&&) = default;

	bool operator==(const AABB&) const;

	[[nodiscard]] bool intersects(const AABB& other) const;
	[[nodiscard]] bool intersects(const Vector3& point) const;
};

// Helper Functions
float deg_to_rad(float degrees);
float rad_to_deg(float radians);

Vector3 deg_to_rad(const Vector3& degrees);
Vector3 rad_to_deg(const Vector3& radians);

uint32_t round_up_to_next_pow_2(uint32_t x);

template <class T>
	requires std::is_integral_v<T>
bool is_power_of_two(T x) {
	return (x & (x - 1)) == 0;
}

constexpr uint32_t raise_to_next_multiple_of(uint32_t val, uint32_t multiple);

Matrix convert_direction_vector_to_rotation_matrix(Vector3 forward);

inline bool is_power_of_two(int n) {
	if (n == 0) {
		return false;
	}

	return ceil(log2(n)) == floor(log2(n));
}

namespace math::matrices {

enum class Axis : uint8_t {
	X = 0,
	Y,
	Z
};

// Row `i` of the upper 3x3, which is axis `i` of the transform.
inline Vector3 get_axis(const Matrix& mat, Axis axis) {
	const Vector4& row = mat[std::to_underlying(axis)];
	return { row.x, row.y, row.z };
}

inline void set_axis(Matrix& m, uint32_t i, const Vector3& axis) {
	m[i] = { axis.x, axis.y, axis.z, m[i].w };
}

// Normalizes each of the three axes, leaving rotation and dropping scale.
inline void remove_scaling(Matrix& m) {
	for (uint32_t i = 0; i < 3; ++i) {
		const Vector3 axis = get_axis(m, static_cast<Axis>(i));
		const real_t length_squared = axis.length_squared();
		set_axis(m, i, length_squared > 0 ? axis / std::sqrt(length_squared) : axis);
	}
}

inline Vector3 get_origin(const Matrix& mat) {
	return mat.translation();
}

} //namespace math::matrices

inline constexpr float small_number = 1.e-4f;
inline constexpr float quaternion_normalize_threshhold = 0.01f;

} //namespace feather
