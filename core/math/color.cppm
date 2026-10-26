module;

#include <rtm/impl/compiler_utils.h>

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "color.gen.h"
#endif

export module feather.core:math.color;

import rtm;
import std.compat;
import :framework.static_string;
import :math.rtm_interop;

export namespace feather {

// A linear RGBA color. Float only whatever the engine's precision: doubling a display quantity
// buys a renderer nothing.
struct Colorf {
	FSTRUCT();

	[[get, set]] float r = 0.0f;
	[[get, set]] float g = 0.0f;
	[[get, set]] float b = 0.0f;
	[[get, set]] float a = 1.0f;

	constexpr Colorf() = default;
	constexpr Colorf(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}

	[[nodiscard]] RTM_FORCE_INLINE rtm::vector4f to_rtm() const { return rtm::vector_set(r, g, b, a); }
	static RTM_FORCE_INLINE Colorf from_rtm(rtm::vector4f v) {
		return { rtm::vector_get_x(v), rtm::vector_get_y(v), rtm::vector_get_z(v), rtm::vector_get_w(v) };
	}

	static constexpr Colorf from_bytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
		return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}

	RTM_FORCE_INLINE Colorf operator+(const Colorf& o) const { return from_rtm(rtm::vector_add(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Colorf operator-(const Colorf& o) const { return from_rtm(rtm::vector_sub(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Colorf operator*(const Colorf& o) const { return from_rtm(rtm::vector_mul(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Colorf operator*(float s) const { return from_rtm(rtm::vector_mul(to_rtm(), s)); }
	RTM_FORCE_INLINE Colorf operator/(float s) const { return from_rtm(rtm::vector_div(to_rtm(), rtm::vector_set(s))); }

	RTM_FORCE_INLINE Colorf& operator+=(const Colorf& o) { return *this = *this + o; }
	RTM_FORCE_INLINE Colorf& operator-=(const Colorf& o) { return *this = *this - o; }
	RTM_FORCE_INLINE Colorf& operator*=(const Colorf& o) { return *this = *this * o; }
	RTM_FORCE_INLINE Colorf& operator*=(float s) { return *this = *this * s; }
	RTM_FORCE_INLINE Colorf& operator/=(float s) { return *this = *this / s; }

	RTM_FORCE_INLINE bool operator==(const Colorf& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
	RTM_FORCE_INLINE bool operator!=(const Colorf& o) const { return !(*this == o); }
	[[nodiscard]] inline bool is_near(const Colorf& o, float tolerance = 1.e-4f) const {
		return rtm::vector_all_near_equal(to_rtm(), o.to_rtm(), tolerance);
	}

	RTM_FORCE_INLINE float& operator[](size_t i) { return (&r)[i]; }
	RTM_FORCE_INLINE const float& operator[](size_t i) const { return (&r)[i]; }

	// Clamped to the unit range a renderer expects after arithmetic.
	[[nodiscard]] inline Colorf saturated() const {
		return {
			std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f), std::clamp(b, 0.0f, 1.0f), std::clamp(a, 0.0f, 1.0f)
		};
	}
	[[nodiscard]] RTM_FORCE_INLINE Colorf lerp(const Colorf& o, float alpha) const {
		return from_rtm(rtm::vector_lerp(to_rtm(), o.to_rtm(), alpha));
	}
	[[nodiscard]] RTM_FORCE_INLINE Colorf with_alpha(float alpha) const { return { r, g, b, alpha }; }

	static const Colorf black;
	static const Colorf white;
	static const Colorf red;
	static const Colorf green;
	static const Colorf blue;
	static const Colorf transparent;
};

static_assert(math::mirrors_rtm_storage<Colorf, float, 4>);

inline constexpr Colorf Colorf::black { 0.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Colorf Colorf::white { 1.0f, 1.0f, 1.0f, 1.0f };
inline constexpr Colorf Colorf::red { 1.0f, 0.0f, 0.0f, 1.0f };
inline constexpr Colorf Colorf::green { 0.0f, 1.0f, 0.0f, 1.0f };
inline constexpr Colorf Colorf::blue { 0.0f, 0.0f, 1.0f, 1.0f };
inline constexpr Colorf Colorf::transparent { 0.0f, 0.0f, 0.0f, 0.0f };

RTM_FORCE_INLINE Colorf operator*(float s, const Colorf& c) {
	return c * s;
}

} //namespace feather

template <> struct std::formatter<feather::Colorf> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Colorf& c, std::format_context& ctx) {
		return std::format_to(ctx.out(), "R={}, G={}, B={}, A={}", c.r, c.g, c.b, c.a);
	}
};
