module;

#include <rtm/impl/compiler_utils.h>

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "vector3.gen.h"
#endif

export module feather.core:math.vector3;

import rtm;
import std.compat;
import :framework.static_string;
import :math.rtm_interop;

export namespace feather {

// A 3-component vector stored as plain reals. Operations load it into RTM's register type,
// compute there and store back.
struct Vector3f {
	FSTRUCT();

	[[get, set]] float x = 0.0f;
	[[get, set]] float y = 0.0f;
	[[get, set]] float z = 0.0f;

	constexpr Vector3f() = default;
	constexpr Vector3f(float x, float y, float z) : x(x), y(y), z(z) {}
	explicit constexpr Vector3f(float value) : x(value), y(value), z(value) {}

	// The unused lanes are zero.
	[[nodiscard]] RTM_FORCE_INLINE rtm::vector4f to_rtm() const { return rtm::vector_set(x, y, z, 0.0f); }
	static RTM_FORCE_INLINE Vector3f from_rtm(rtm::vector4f v) {
		return { rtm::vector_get_x(v), rtm::vector_get_y(v), rtm::vector_get_z(v) };
	}

	RTM_FORCE_INLINE Vector3f operator+(const Vector3f& o) const { return from_rtm(rtm::vector_add(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3f operator-(const Vector3f& o) const { return from_rtm(rtm::vector_sub(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3f operator*(const Vector3f& o) const { return from_rtm(rtm::vector_mul(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3f operator/(const Vector3f& o) const { return from_rtm(rtm::vector_div(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3f operator*(float s) const { return from_rtm(rtm::vector_mul(to_rtm(), s)); }
	RTM_FORCE_INLINE Vector3f operator/(float s) const { return from_rtm(rtm::vector_div(to_rtm(), rtm::vector_set(s))); }
	RTM_FORCE_INLINE Vector3f operator-() const { return from_rtm(rtm::vector_neg(to_rtm())); }
	RTM_FORCE_INLINE Vector3f operator+() const { return *this; }

	RTM_FORCE_INLINE Vector3f& operator+=(const Vector3f& o) { return *this = *this + o; }
	RTM_FORCE_INLINE Vector3f& operator-=(const Vector3f& o) { return *this = *this - o; }
	RTM_FORCE_INLINE Vector3f& operator*=(const Vector3f& o) { return *this = *this * o; }
	RTM_FORCE_INLINE Vector3f& operator/=(const Vector3f& o) { return *this = *this / o; }
	RTM_FORCE_INLINE Vector3f& operator*=(float s) { return *this = *this * s; }
	RTM_FORCE_INLINE Vector3f& operator/=(float s) { return *this = *this / s; }

	RTM_FORCE_INLINE bool operator==(const Vector3f& o) const { return x == o.x && y == o.y && z == o.z; }
	RTM_FORCE_INLINE bool operator!=(const Vector3f& o) const { return !(*this == o); }
	// Component-wise within `tolerance`; operator== stays exact.
	[[nodiscard]] inline bool is_near(const Vector3f& o, float tolerance = 1.e-4f) const {
		return std::abs(x - o.x) <= tolerance && std::abs(y - o.y) <= tolerance && std::abs(z - o.z) <= tolerance;
	}

	RTM_FORCE_INLINE float& operator[](size_t i) { return (&x)[i]; }
	RTM_FORCE_INLINE const float& operator[](size_t i) const { return (&x)[i]; }

	[[nodiscard]] RTM_FORCE_INLINE float length() const { return rtm::vector_length3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE float length_squared() const { return rtm::vector_length_squared3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE float dot(const Vector3f& o) const { return rtm::vector_dot3(to_rtm(), o.to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f cross(const Vector3f& o) const { return from_rtm(rtm::vector_cross3(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f normalized() const { return from_rtm(rtm::vector_normalize3(to_rtm())); }
	RTM_FORCE_INLINE void normalize() { *this = normalized(); }
	[[nodiscard]] inline float distance(const Vector3f& o) const { return (*this - o).length(); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f min(const Vector3f& o) const { return from_rtm(rtm::vector_min(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f max(const Vector3f& o) const { return from_rtm(rtm::vector_max(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f abs() const { return from_rtm(rtm::vector_abs(to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3f lerp(const Vector3f& o, float alpha) const {
		return from_rtm(rtm::vector_lerp(to_rtm(), o.to_rtm(), alpha));
	}
	// Component-wise 1/x, leaving components within `tolerance` of zero at zero.
	[[nodiscard]] inline Vector3f reciprocal_safe(float tolerance = 1.e-4f) const {
		return { std::abs(x) <= tolerance ? 0.0f : 1.0f / x,
				 std::abs(y) <= tolerance ? 0.0f : 1.0f / y,
				 std::abs(z) <= tolerance ? 0.0f : 1.0f / z };
	}
	[[nodiscard]] RTM_FORCE_INLINE bool any_component_less_equal(float value) const { return x <= value || y <= value || z <= value; }

	// Right-handed: +X right, +Y up, -Z forward.
	static const Vector3f zero;
	static const Vector3f one;
	static const Vector3f right;
	static const Vector3f left;
	static const Vector3f up;
	static const Vector3f down;
	static const Vector3f forward;
	static const Vector3f backward;
};

static_assert(math::mirrors_rtm_storage<Vector3f, float, 3>);

inline constexpr Vector3f Vector3f::zero { 0.0f, 0.0f, 0.0f };
inline constexpr Vector3f Vector3f::one { 1.0f, 1.0f, 1.0f };
inline constexpr Vector3f Vector3f::right { 1.0f, 0.0f, 0.0f };
inline constexpr Vector3f Vector3f::left { -1.0f, 0.0f, 0.0f };
inline constexpr Vector3f Vector3f::up { 0.0f, 1.0f, 0.0f };
inline constexpr Vector3f Vector3f::down { 0.0f, -1.0f, 0.0f };
inline constexpr Vector3f Vector3f::forward { 0.0f, 0.0f, -1.0f };
inline constexpr Vector3f Vector3f::backward { 0.0f, 0.0f, 1.0f };

RTM_FORCE_INLINE Vector3f operator*(float s, const Vector3f& v) {
	return v * s;
}

// The double-precision counterpart, kept a separate type so both stay reflectable.
struct Vector3d {
	FSTRUCT();

	[[get, set]] double x = 0.0;
	[[get, set]] double y = 0.0;
	[[get, set]] double z = 0.0;

	constexpr Vector3d() = default;
	constexpr Vector3d(double x, double y, double z) : x(x), y(y), z(z) {}
	explicit constexpr Vector3d(double value) : x(value), y(value), z(value) {}

	// The unused lanes are zero.
	[[nodiscard]] RTM_FORCE_INLINE rtm::vector4d to_rtm() const { return rtm::vector_set(x, y, z, 0.0); }
	static RTM_FORCE_INLINE Vector3d from_rtm(rtm::vector4d v) {
		return { rtm::vector_get_x(v), rtm::vector_get_y(v), rtm::vector_get_z(v) };
	}

	RTM_FORCE_INLINE Vector3d operator+(const Vector3d& o) const { return from_rtm(rtm::vector_add(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3d operator-(const Vector3d& o) const { return from_rtm(rtm::vector_sub(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3d operator*(const Vector3d& o) const { return from_rtm(rtm::vector_mul(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3d operator/(const Vector3d& o) const { return from_rtm(rtm::vector_div(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Vector3d operator*(double s) const { return from_rtm(rtm::vector_mul(to_rtm(), s)); }
	RTM_FORCE_INLINE Vector3d operator/(double s) const { return from_rtm(rtm::vector_div(to_rtm(), rtm::vector_set(s))); }
	RTM_FORCE_INLINE Vector3d operator-() const { return from_rtm(rtm::vector_neg(to_rtm())); }
	RTM_FORCE_INLINE Vector3d operator+() const { return *this; }

	RTM_FORCE_INLINE Vector3d& operator+=(const Vector3d& o) { return *this = *this + o; }
	RTM_FORCE_INLINE Vector3d& operator-=(const Vector3d& o) { return *this = *this - o; }
	RTM_FORCE_INLINE Vector3d& operator*=(const Vector3d& o) { return *this = *this * o; }
	RTM_FORCE_INLINE Vector3d& operator/=(const Vector3d& o) { return *this = *this / o; }
	RTM_FORCE_INLINE Vector3d& operator*=(double s) { return *this = *this * s; }
	RTM_FORCE_INLINE Vector3d& operator/=(double s) { return *this = *this / s; }

	RTM_FORCE_INLINE bool operator==(const Vector3d& o) const { return x == o.x && y == o.y && z == o.z; }
	RTM_FORCE_INLINE bool operator!=(const Vector3d& o) const { return !(*this == o); }
	// Component-wise within `tolerance`; operator== stays exact.
	[[nodiscard]] inline bool is_near(const Vector3d& o, double tolerance = 1.e-8) const {
		return std::abs(x - o.x) <= tolerance && std::abs(y - o.y) <= tolerance && std::abs(z - o.z) <= tolerance;
	}

	RTM_FORCE_INLINE double& operator[](size_t i) { return (&x)[i]; }
	RTM_FORCE_INLINE const double& operator[](size_t i) const { return (&x)[i]; }

	[[nodiscard]] RTM_FORCE_INLINE double length() const { return rtm::vector_length3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE double length_squared() const { return rtm::vector_length_squared3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE double dot(const Vector3d& o) const { return rtm::vector_dot3(to_rtm(), o.to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d cross(const Vector3d& o) const { return from_rtm(rtm::vector_cross3(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d normalized() const { return from_rtm(rtm::vector_normalize3(to_rtm())); }
	RTM_FORCE_INLINE void normalize() { *this = normalized(); }
	[[nodiscard]] inline double distance(const Vector3d& o) const { return (*this - o).length(); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d min(const Vector3d& o) const { return from_rtm(rtm::vector_min(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d max(const Vector3d& o) const { return from_rtm(rtm::vector_max(to_rtm(), o.to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d abs() const { return from_rtm(rtm::vector_abs(to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector3d lerp(const Vector3d& o, double alpha) const {
		return from_rtm(rtm::vector_lerp(to_rtm(), o.to_rtm(), alpha));
	}
	// Component-wise 1/x, leaving components within `tolerance` of zero at zero.
	[[nodiscard]] inline Vector3d reciprocal_safe(double tolerance = 1.e-8) const {
		return { std::abs(x) <= tolerance ? 0.0 : 1.0 / x,
				 std::abs(y) <= tolerance ? 0.0 : 1.0 / y,
				 std::abs(z) <= tolerance ? 0.0 : 1.0 / z };
	}
	[[nodiscard]] RTM_FORCE_INLINE bool any_component_less_equal(double value) const { return x <= value || y <= value || z <= value; }

	static const Vector3d zero;
	static const Vector3d one;
	static const Vector3d right;
	static const Vector3d left;
	static const Vector3d up;
	static const Vector3d down;
	static const Vector3d forward;
	static const Vector3d backward;
};

static_assert(math::mirrors_rtm_storage<Vector3d, double, 3>);

inline constexpr Vector3d Vector3d::zero { 0.0, 0.0, 0.0 };
inline constexpr Vector3d Vector3d::one { 1.0, 1.0, 1.0 };
inline constexpr Vector3d Vector3d::right { 1.0, 0.0, 0.0 };
inline constexpr Vector3d Vector3d::left { -1.0, 0.0, 0.0 };
inline constexpr Vector3d Vector3d::up { 0.0, 1.0, 0.0 };
inline constexpr Vector3d Vector3d::down { 0.0, -1.0, 0.0 };
inline constexpr Vector3d Vector3d::forward { 0.0, 0.0, -1.0 };
inline constexpr Vector3d Vector3d::backward { 0.0, 0.0, 1.0 };

RTM_FORCE_INLINE Vector3d operator*(double s, const Vector3d& v) {
	return v * s;
}

} //namespace feather

template <> struct std::formatter<feather::Vector3f> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Vector3f& v, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}, Z={}", v.x, v.y, v.z);
	}
};

template <> struct std::formatter<feather::Vector3d> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Vector3d& v, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}, Z={}", v.x, v.y, v.z);
	}
};
