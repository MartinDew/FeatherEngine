module;

#include <rtm/impl/compiler_utils.h>

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "vector2.gen.h"
#endif

export module feather.core:math.vector2;

import rtm;
import std.compat;
import :framework.static_string;
import :math.rtm_interop;

export namespace feather {

// A 2-component vector stored as plain reals. Operations load it into RTM's register type,
// compute there and store back.
struct Vector2f {
	FSTRUCT();

	[[get, set]] float x = 0.0f;
	[[get, set]] float y = 0.0f;

	constexpr Vector2f() = default;
	constexpr Vector2f(float x, float y) : x(x), y(y) {}
	explicit constexpr Vector2f(float value) : x(value), y(value) {}

	// The unused lanes are zero.
	[[nodiscard]] RTM_FORCE_INLINE rtm::vector4f to_rtm() const { return rtm::vector_set(x, y, 0.0f, 0.0f); }
	static RTM_FORCE_INLINE Vector2f from_rtm(rtm::vector4f v) {
		return { rtm::vector_get_x(v), rtm::vector_get_y(v) };
	}

	RTM_FORCE_INLINE Vector2f operator+(const Vector2f& o) const {
		return from_rtm(rtm::vector_add(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2f operator-(const Vector2f& o) const {
		return from_rtm(rtm::vector_sub(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2f operator*(const Vector2f& o) const {
		return from_rtm(rtm::vector_mul(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2f operator/(const Vector2f& o) const {
		return from_rtm(rtm::vector_div(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2f operator*(float s) const { return from_rtm(rtm::vector_mul(to_rtm(), s)); }
	RTM_FORCE_INLINE Vector2f operator/(float s) const {
		return from_rtm(rtm::vector_div(to_rtm(), rtm::vector_set(s)));
	}
	RTM_FORCE_INLINE Vector2f operator-() const { return from_rtm(rtm::vector_neg(to_rtm())); }
	RTM_FORCE_INLINE Vector2f operator+() const { return *this; }

	RTM_FORCE_INLINE Vector2f& operator+=(const Vector2f& o) { return *this = *this + o; }
	RTM_FORCE_INLINE Vector2f& operator-=(const Vector2f& o) { return *this = *this - o; }
	RTM_FORCE_INLINE Vector2f& operator*=(const Vector2f& o) { return *this = *this * o; }
	RTM_FORCE_INLINE Vector2f& operator/=(const Vector2f& o) { return *this = *this / o; }
	RTM_FORCE_INLINE Vector2f& operator*=(float s) { return *this = *this * s; }
	RTM_FORCE_INLINE Vector2f& operator/=(float s) { return *this = *this / s; }

	RTM_FORCE_INLINE bool operator==(const Vector2f& o) const { return x == o.x && y == o.y; }
	RTM_FORCE_INLINE bool operator!=(const Vector2f& o) const { return !(*this == o); }
	// Component-wise within `tolerance`; operator== stays exact.
	[[nodiscard]] inline bool is_near(const Vector2f& o, float tolerance = 1.e-4f) const {
		return std::abs(x - o.x) <= tolerance && std::abs(y - o.y) <= tolerance;
	}

	RTM_FORCE_INLINE float& operator[](size_t i) { return (&x)[i]; }
	RTM_FORCE_INLINE const float& operator[](size_t i) const { return (&x)[i]; }

	[[nodiscard]] RTM_FORCE_INLINE float length() const { return rtm::vector_length3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE float length_squared() const { return rtm::vector_length_squared3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE float dot(const Vector2f& o) const { return rtm::vector_dot3(to_rtm(), o.to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE Vector2f normalized() const { return from_rtm(rtm::vector_normalize3(to_rtm())); }
	RTM_FORCE_INLINE void normalize() { *this = normalized(); }
	[[nodiscard]] inline float distance(const Vector2f& o) const { return (*this - o).length(); }
	[[nodiscard]] RTM_FORCE_INLINE Vector2f min(const Vector2f& o) const {
		return from_rtm(rtm::vector_min(to_rtm(), o.to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector2f max(const Vector2f& o) const {
		return from_rtm(rtm::vector_max(to_rtm(), o.to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector2f abs() const { return from_rtm(rtm::vector_abs(to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector2f lerp(const Vector2f& o, float alpha) const {
		return from_rtm(rtm::vector_lerp(to_rtm(), o.to_rtm(), alpha));
	}

	static const Vector2f zero;
	static const Vector2f one;
	static const Vector2f right;
	static const Vector2f left;
	static const Vector2f up;
	static const Vector2f down;
};

static_assert(math::mirrors_rtm_storage<Vector2f, float, 2>);

inline constexpr Vector2f Vector2f::zero { 0.0f, 0.0f };
inline constexpr Vector2f Vector2f::one { 1.0f, 1.0f };
inline constexpr Vector2f Vector2f::right { 1.0f, 0.0f };
inline constexpr Vector2f Vector2f::left { -1.0f, 0.0f };
inline constexpr Vector2f Vector2f::up { 0.0f, 1.0f };
inline constexpr Vector2f Vector2f::down { 0.0f, -1.0f };

RTM_FORCE_INLINE Vector2f operator*(float s, const Vector2f& v) {
	return v * s;
}

// The double-precision counterpart, kept a separate type so both stay reflectable.
struct Vector2d {
	FSTRUCT();

	[[get, set]] double x = 0.0;
	[[get, set]] double y = 0.0;

	constexpr Vector2d() = default;
	constexpr Vector2d(double x, double y) : x(x), y(y) {}
	explicit constexpr Vector2d(double value) : x(value), y(value) {}

	// The unused lanes are zero.
	[[nodiscard]] RTM_FORCE_INLINE rtm::vector4d to_rtm() const { return rtm::vector_set(x, y, 0.0, 0.0); }
	static RTM_FORCE_INLINE Vector2d from_rtm(rtm::vector4d v) {
		return { rtm::vector_get_x(v), rtm::vector_get_y(v) };
	}

	RTM_FORCE_INLINE Vector2d operator+(const Vector2d& o) const {
		return from_rtm(rtm::vector_add(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2d operator-(const Vector2d& o) const {
		return from_rtm(rtm::vector_sub(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2d operator*(const Vector2d& o) const {
		return from_rtm(rtm::vector_mul(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2d operator/(const Vector2d& o) const {
		return from_rtm(rtm::vector_div(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Vector2d operator*(double s) const { return from_rtm(rtm::vector_mul(to_rtm(), s)); }
	RTM_FORCE_INLINE Vector2d operator/(double s) const {
		return from_rtm(rtm::vector_div(to_rtm(), rtm::vector_set(s)));
	}
	RTM_FORCE_INLINE Vector2d operator-() const { return from_rtm(rtm::vector_neg(to_rtm())); }
	RTM_FORCE_INLINE Vector2d operator+() const { return *this; }

	RTM_FORCE_INLINE Vector2d& operator+=(const Vector2d& o) { return *this = *this + o; }
	RTM_FORCE_INLINE Vector2d& operator-=(const Vector2d& o) { return *this = *this - o; }
	RTM_FORCE_INLINE Vector2d& operator*=(const Vector2d& o) { return *this = *this * o; }
	RTM_FORCE_INLINE Vector2d& operator/=(const Vector2d& o) { return *this = *this / o; }
	RTM_FORCE_INLINE Vector2d& operator*=(double s) { return *this = *this * s; }
	RTM_FORCE_INLINE Vector2d& operator/=(double s) { return *this = *this / s; }

	RTM_FORCE_INLINE bool operator==(const Vector2d& o) const { return x == o.x && y == o.y; }
	RTM_FORCE_INLINE bool operator!=(const Vector2d& o) const { return !(*this == o); }
	// Component-wise within `tolerance`; operator== stays exact.
	[[nodiscard]] inline bool is_near(const Vector2d& o, double tolerance = 1.e-8) const {
		return std::abs(x - o.x) <= tolerance && std::abs(y - o.y) <= tolerance;
	}

	RTM_FORCE_INLINE double& operator[](size_t i) { return (&x)[i]; }
	RTM_FORCE_INLINE const double& operator[](size_t i) const { return (&x)[i]; }

	[[nodiscard]] RTM_FORCE_INLINE double length() const { return rtm::vector_length3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE double length_squared() const { return rtm::vector_length_squared3(to_rtm()); }
	[[nodiscard]] RTM_FORCE_INLINE double dot(const Vector2d& o) const {
		return rtm::vector_dot3(to_rtm(), o.to_rtm());
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector2d normalized() const { return from_rtm(rtm::vector_normalize3(to_rtm())); }
	RTM_FORCE_INLINE void normalize() { *this = normalized(); }
	[[nodiscard]] inline double distance(const Vector2d& o) const { return (*this - o).length(); }
	[[nodiscard]] RTM_FORCE_INLINE Vector2d min(const Vector2d& o) const {
		return from_rtm(rtm::vector_min(to_rtm(), o.to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector2d max(const Vector2d& o) const {
		return from_rtm(rtm::vector_max(to_rtm(), o.to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector2d abs() const { return from_rtm(rtm::vector_abs(to_rtm())); }
	[[nodiscard]] RTM_FORCE_INLINE Vector2d lerp(const Vector2d& o, double alpha) const {
		return from_rtm(rtm::vector_lerp(to_rtm(), o.to_rtm(), alpha));
	}

	static const Vector2d zero;
	static const Vector2d one;
	static const Vector2d right;
	static const Vector2d left;
	static const Vector2d up;
	static const Vector2d down;
};

static_assert(math::mirrors_rtm_storage<Vector2d, double, 2>);

inline constexpr Vector2d Vector2d::zero { 0.0, 0.0 };
inline constexpr Vector2d Vector2d::one { 1.0, 1.0 };
inline constexpr Vector2d Vector2d::right { 1.0, 0.0 };
inline constexpr Vector2d Vector2d::left { -1.0, 0.0 };
inline constexpr Vector2d Vector2d::up { 0.0, 1.0 };
inline constexpr Vector2d Vector2d::down { 0.0, -1.0 };

RTM_FORCE_INLINE Vector2d operator*(double s, const Vector2d& v) {
	return v * s;
}

} //namespace feather

template <> struct std::formatter<feather::Vector2f> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Vector2f& v, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}", v.x, v.y);
	}
};

template <> struct std::formatter<feather::Vector2d> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Vector2d& v, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}", v.x, v.y);
	}
};
