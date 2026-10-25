module;

#include <rtm/quatd.h>
#include <rtm/quatf.h>

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "quaternion.gen.h"
#endif

export module feather.core:math.quaternion;

import std.compat;
import :framework.static_string;
import :math.rtm_interop;
import :math.vector3;

export namespace feather {

struct Matrix4x4f;

// A rotation stored as four plain reals, laid out like a 4-component vector so it reaches RTM's
// register type without a conversion step.
struct Quaternionf {
	FSTRUCT();

	[[get, set]] float x = 0.0f;
	[[get, set]] float y = 0.0f;
	[[get, set]] float z = 0.0f;
	[[get, set]] float w = 1.0f;

	constexpr Quaternionf() = default;
	constexpr Quaternionf(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	[[nodiscard]] rtm::quatf to_rtm() const { return rtm::quat_set(x, y, z, w); }
	static Quaternionf from_rtm(rtm::quatf q) {
		return { rtm::quat_get_x(q), rtm::quat_get_y(q), rtm::quat_get_z(q), rtm::quat_get_w(q) };
	}

	static Quaternionf from_axis_angle(const Vector3f& axis, float radians) {
		return from_rtm(rtm::quat_from_axis_angle(axis.to_rtm(), radians));
	}
	// Radians; yaw turns about +Y, pitch about +X, roll about +Z, applied roll, then pitch, then yaw.
	static Quaternionf from_euler(float yaw, float pitch, float roll) {
		return from_axis_angle(Vector3f::backward, roll) * from_axis_angle(Vector3f::right, pitch) *
				from_axis_angle(Vector3f::up, yaw);
	}
	// The rotation part of a matrix, which must already be free of scale.
	static Quaternionf from_matrix(const Matrix4x4f& m);

	// `a * b` applies a, then b.
	Quaternionf operator*(const Quaternionf& o) const { return from_rtm(rtm::quat_mul(to_rtm(), o.to_rtm())); }
	Quaternionf& operator*=(const Quaternionf& o) { return *this = *this * o; }
	Quaternionf operator-() const { return { -x, -y, -z, -w }; }

	// Rotates a vector by this rotation.
	Vector3f operator*(const Vector3f& v) const {
		return Vector3f::from_rtm(rtm::quat_mul_vector3(v.to_rtm(), to_rtm()));
	}

	bool operator==(const Quaternionf& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	bool operator!=(const Quaternionf& o) const { return !(*this == o); }
	[[nodiscard]] bool is_near(const Quaternionf& o, float tolerance = 1.e-4f) const {
		return rtm::quat_near_equal(to_rtm(), o.to_rtm(), tolerance);
	}

	[[nodiscard]] float length() const { return rtm::quat_length(to_rtm()); }
	[[nodiscard]] float length_squared() const { return rtm::quat_length_squared(to_rtm()); }
	[[nodiscard]] float dot(const Quaternionf& o) const { return rtm::quat_dot(to_rtm(), o.to_rtm()); }
	[[nodiscard]] Quaternionf normalized() const { return from_rtm(rtm::quat_normalize(to_rtm())); }
	void normalize() { *this = normalized(); }
	// The inverse of a unit rotation, and the cheap form of inverse() when the rotation is known to be one.
	[[nodiscard]] Quaternionf conjugate() const { return from_rtm(rtm::quat_conjugate(to_rtm())); }
	[[nodiscard]] Quaternionf inverse() const {
		const Quaternionf c = conjugate();
		const float inv_length_squared = 1.0f / length_squared();
		return {
			c.x * inv_length_squared, c.y * inv_length_squared, c.z * inv_length_squared, c.w * inv_length_squared
		};
	}
	[[nodiscard]] bool is_normalized(float threshold = 1.e-4f) const {
		return rtm::quat_is_normalized(to_rtm(), threshold);
	}
	[[nodiscard]] Vector3f axis() const { return Vector3f::from_rtm(rtm::quat_get_axis(to_rtm())); }
	[[nodiscard]] float angle() const { return rtm::quat_get_angle(to_rtm()); }
	[[nodiscard]] Quaternionf lerp(const Quaternionf& o, float alpha) const {
		return from_rtm(rtm::quat_lerp(to_rtm(), o.to_rtm(), alpha));
	}
	[[nodiscard]] Quaternionf slerp(const Quaternionf& o, float alpha) const {
		return from_rtm(rtm::quat_slerp(to_rtm(), o.to_rtm(), alpha));
	}

	static const Quaternionf identity;
};

static_assert(math::mirrors_rtm_storage<Quaternionf, float, 4>);

inline constexpr Quaternionf Quaternionf::identity { 0.0f, 0.0f, 0.0f, 1.0f };

// The double-precision counterpart, kept a separate type so both stay reflectable.
struct Quaterniond {
	FSTRUCT();

	[[get, set]] double x = 0.0;
	[[get, set]] double y = 0.0;
	[[get, set]] double z = 0.0;
	[[get, set]] double w = 1.0;

	constexpr Quaterniond() = default;
	constexpr Quaterniond(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {}

	[[nodiscard]] rtm::quatd to_rtm() const { return rtm::quat_set(x, y, z, w); }
	static Quaterniond from_rtm(rtm::quatd q) {
		return { rtm::quat_get_x(q), rtm::quat_get_y(q), rtm::quat_get_z(q), rtm::quat_get_w(q) };
	}

	static Quaterniond from_axis_angle(const Vector3d& axis, double radians) {
		return from_rtm(rtm::quat_from_axis_angle(axis.to_rtm(), radians));
	}
	static Quaterniond from_euler(double yaw, double pitch, double roll) {
		return from_axis_angle(Vector3d::backward, roll) * from_axis_angle(Vector3d::right, pitch) *
				from_axis_angle(Vector3d::up, yaw);
	}

	Quaterniond operator*(const Quaterniond& o) const { return from_rtm(rtm::quat_mul(to_rtm(), o.to_rtm())); }
	Quaterniond& operator*=(const Quaterniond& o) { return *this = *this * o; }
	Quaterniond operator-() const { return { -x, -y, -z, -w }; }
	Vector3d operator*(const Vector3d& v) const {
		return Vector3d::from_rtm(rtm::quat_mul_vector3(v.to_rtm(), to_rtm()));
	}

	bool operator==(const Quaterniond& o) const { return x == o.x && y == o.y && z == o.z && w == o.w; }
	bool operator!=(const Quaterniond& o) const { return !(*this == o); }
	[[nodiscard]] bool is_near(const Quaterniond& o, double tolerance = 1.e-8) const {
		return rtm::quat_near_equal(to_rtm(), o.to_rtm(), tolerance);
	}

	[[nodiscard]] double length() const { return rtm::quat_length(to_rtm()); }
	[[nodiscard]] double length_squared() const { return rtm::quat_length_squared(to_rtm()); }
	[[nodiscard]] double dot(const Quaterniond& o) const { return rtm::quat_dot(to_rtm(), o.to_rtm()); }
	[[nodiscard]] Quaterniond normalized() const { return from_rtm(rtm::quat_normalize(to_rtm())); }
	void normalize() { *this = normalized(); }
	[[nodiscard]] Quaterniond conjugate() const { return from_rtm(rtm::quat_conjugate(to_rtm())); }
	[[nodiscard]] Quaterniond inverse() const {
		const Quaterniond c = conjugate();
		const double inv_length_squared = 1.0 / length_squared();
		return {
			c.x * inv_length_squared, c.y * inv_length_squared, c.z * inv_length_squared, c.w * inv_length_squared
		};
	}
	[[nodiscard]] bool is_normalized(double threshold = 1.e-8) const {
		return rtm::quat_is_normalized(to_rtm(), threshold);
	}
	[[nodiscard]] Vector3d axis() const { return Vector3d::from_rtm(rtm::quat_get_axis(to_rtm())); }
	[[nodiscard]] double angle() const { return rtm::quat_get_angle(to_rtm()); }
	[[nodiscard]] Quaterniond lerp(const Quaterniond& o, double alpha) const {
		return from_rtm(rtm::quat_lerp(to_rtm(), o.to_rtm(), alpha));
	}
	[[nodiscard]] Quaterniond slerp(const Quaterniond& o, double alpha) const {
		return from_rtm(rtm::quat_slerp(to_rtm(), o.to_rtm(), alpha));
	}

	static const Quaterniond identity;
};

static_assert(math::mirrors_rtm_storage<Quaterniond, double, 4>);

inline constexpr Quaterniond Quaterniond::identity { 0.0, 0.0, 0.0, 1.0 };

} //namespace feather

template <> struct std::formatter<feather::Quaternionf> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static auto format(const feather::Quaternionf& q, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}, Z={}, W={}", q.x, q.y, q.z, q.w);
	}
};

template <> struct std::formatter<feather::Quaterniond> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static auto format(const feather::Quaterniond& q, std::format_context& ctx) {
		return std::format_to(ctx.out(), "X={}, Y={}, Z={}, W={}", q.x, q.y, q.z, q.w);
	}
};
