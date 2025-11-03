#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>

#include <utility>

namespace feather {

using Matrix = DirectX::SimpleMath::Matrix;
using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;
using Color = DirectX::SimpleMath::Color;
using Quaternion = DirectX::SimpleMath::Quaternion;
using UByteColor = std::array<uint8_t, 4>;

// // TODO: Add better support for math and conversion operations using these types
using UIntVector4 = DirectX::XMUINT4;
using UIntVector3 = DirectX::XMUINT3;
using UIntVector2 = DirectX::XMUINT2;

using IntVector4 = DirectX::XMINT4;
using IntVector3 = DirectX::XMINT3;
using IntVector2 = DirectX::XMINT2;

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

constexpr uint32_t RaiseToNextMultipleOf(uint32_t val, uint32_t multiple);

Matrix convert_direction_vector_to_rotation_matrix(Vector3 forward);

inline bool is_power_of_two(int n) {
	if (n == 0)
		return false;

	return ceil(log2(n)) == floor(log2(n));
}

namespace Math::Matrices {

inline void RemoveScaling(Matrix& m) {
	const float squareSum0 = (m.m[0][0] * m.m[0][0]) + (m.m[0][1] * m.m[0][1]) + (m.m[0][2] * m.m[0][2]);
	const float squareSum1 = (m.m[1][0] * m.m[1][0]) + (m.m[1][1] * m.m[1][1]) + (m.m[1][2] * m.m[1][2]);
	const float squareSum2 = (m.m[2][0] * m.m[2][0]) + (m.m[2][1] * m.m[2][1]) + (m.m[2][2] * m.m[2][2]);
	const float scale0 = squareSum0 >= 0 ? std::sqrt(squareSum0) : 1;
	const float scale1 = squareSum1 >= 0 ? std::sqrt(squareSum1) : 1;
	const float scale2 = squareSum2 >= 0 ? std::sqrt(squareSum2) : 1;
	m.m[0][0] *= scale0;
	m.m[0][1] *= scale0;
	m.m[0][2] *= scale0;
	m.m[1][0] *= scale1;
	m.m[1][1] *= scale1;
	m.m[1][2] *= scale1;
	m.m[2][0] *= scale2;
	m.m[2][1] *= scale2;
	m.m[2][2] *= scale2;
}

inline void set_axis(Matrix& m, uint32_t i, const Vector3& axis) {
	m.m[i][0] = axis.x;
	m.m[i][1] = axis.y;
	m.m[i][2] = axis.z;
}

enum class Axis : uint8_t {
	X = 0,
	Y,
	Z
};

inline Vector3 get_axis(const Matrix& mat, Axis axis) {
	const uint8_t i = std::to_underlying(axis);
	return { mat.m[i][0], mat.m[i][1], mat.m[i][2] };
}

inline Vector3 get_origin(const Matrix& mat) {
	return { mat.m[3][0], mat.m[3][1], mat.m[3][2] };
}

} //namespace Math::Matrices

inline constexpr float SMALL_NUMBER = 1.e-4f;
inline constexpr float QUATERNION_NORMALIZE_THRESHHOLD = 0.01f;
//
// template <>
// struct std::formatter<Quaternion> {
// 	constexpr auto parse(std::format_parse_context& ctx) {
// 		return ctx.begin();
// 	}
//
// 	auto format(const Quaternion& quat, std::format_context& ctx) const {
// 		return std::format_to(ctx.out(), "X={}, Y={}, Z={}, W={}", quat.x, quat.y, quat.z, quat.w);
// 	}
// };
//
// template <>
// struct std::formatter<Vector4> {
// 	constexpr auto parse(std::format_parse_context& ctx) {
// 		return ctx.begin();
// 	}
//
// 	auto format(const Vector4& vec, std::format_context& ctx) const {
// 		return std::format_to(ctx.out(), "X={}, Y={}, Z={}, W={}", vec.x, vec.y, vec.z, vec.w);
// 	}
// };
//
// template <>
// struct std::formatter<Vector3> {
// 	constexpr auto parse(std::format_parse_context& ctx) {
// 		return ctx.begin();
// 	}
//
// 	auto format(const Vector3& vec, std::format_context& ctx) const {
// 		return std::format_to(ctx.out(), "X={}, Y={}, Z={}", vec.x, vec.y, vec.z);
// 	}
// };
//
// template <>
// struct std::formatter<Vector2> {
// 	constexpr auto parse(std::format_parse_context& ctx) {
// 		return ctx.begin();
// 	}
//
// 	auto format(const Vector2& vec, std::format_context& ctx) const {
// 		return std::format_to(ctx.out(), "X={}, Y={}", vec.x, vec.y);
// 	}
// };
//
// template <>
// struct std::formatter<Color> {
// 	constexpr auto parse(std::format_parse_context& ctx) {
// 		return ctx.begin();
// 	}
//
// 	auto format(const Color& col, std::format_context& ctx) const {
// 		return std::format_to(ctx.out(), "R={}, G={}, B={}, A={}", col.x, col.y, col.z, col.w);
// 	}
// };

} //namespace feather