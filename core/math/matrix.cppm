module;

#include <rtm/impl/compiler_utils.h>

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "matrix.gen.h"
#endif

export module feather.core:math.matrix;

import rtm;
import std.compat;
import :framework.static_string;
import :math.quaternion;
import :math.vector3;
import :math.vector4;

export namespace feather {

// A 4x4 transformation matrix stored as four rows, for row vectors (`v * M`): rows 0-2 are the
// basis axes and row 3 the translation, the layout RTM and the GPU uploads both use.
struct Matrix4x4f {
	FSTRUCT();

	Vector4f x_axis { 1.0f, 0.0f, 0.0f, 0.0f };
	Vector4f y_axis { 0.0f, 1.0f, 0.0f, 0.0f };
	Vector4f z_axis { 0.0f, 0.0f, 1.0f, 0.0f };
	Vector4f w_axis { 0.0f, 0.0f, 0.0f, 1.0f };

	constexpr Matrix4x4f() = default;
	constexpr Matrix4x4f(const Vector4f& x, const Vector4f& y, const Vector4f& z, const Vector4f& w)
			: x_axis(x)
			, y_axis(y)
			, z_axis(z)
			, w_axis(w) {}

	[[nodiscard]] RTM_FORCE_INLINE rtm::matrix4x4f to_rtm() const {
		return rtm::matrix4x4f { x_axis.to_rtm(), y_axis.to_rtm(), z_axis.to_rtm(), w_axis.to_rtm() };
	}
	static RTM_FORCE_INLINE Matrix4x4f from_rtm(const rtm::matrix4x4f& m) {
		return { Vector4f::from_rtm(m.x_axis),
				 Vector4f::from_rtm(m.y_axis),
				 Vector4f::from_rtm(m.z_axis),
				 Vector4f::from_rtm(m.w_axis) };
	}
	// An affine 3x4 leaves its W lanes undefined; they are set to the affine row's 0, 0, 0, 1.
	static RTM_FORCE_INLINE Matrix4x4f from_rtm(const rtm::matrix3x4f& m) {
		return { Vector4f::from_rtm(rtm::vector_set_w(m.x_axis, 0.0f)),
				 Vector4f::from_rtm(rtm::vector_set_w(m.y_axis, 0.0f)),
				 Vector4f::from_rtm(rtm::vector_set_w(m.z_axis, 0.0f)),
				 Vector4f::from_rtm(rtm::vector_set_w(m.w_axis, 1.0f)) };
	}

	static inline Matrix4x4f from_translation(const Vector3f& t) {
		Matrix4x4f m;
		m.w_axis = { t.x, t.y, t.z, 1.0f };
		return m;
	}
	static inline Matrix4x4f from_rotation(const Quaternionf& r) {
		return from_rtm(rtm::matrix_from_qvv(r.to_rtm(), rtm::vector_zero(), rtm::vector_set(1.0f)));
	}
	static inline Matrix4x4f from_scale(const Vector3f& s) {
		Matrix4x4f m;
		m.x_axis = { s.x, 0.0f, 0.0f, 0.0f };
		m.y_axis = { 0.0f, s.y, 0.0f, 0.0f };
		m.z_axis = { 0.0f, 0.0f, s.z, 0.0f };
		return m;
	}
	// Scale, then rotate, then translate.
	static inline Matrix4x4f from_transform(const Vector3f& translation, const Quaternionf& rotation, const Vector3f& scale) {
		return from_rtm(rtm::matrix_from_qvv(rotation.to_rtm(), translation.to_rtm(), scale.to_rtm()));
	}

	// Right-handed, looking down -Z, depth mapped to [0, 1]. RTM's own camera helpers are
	// left-handed, so these are written out here.
	static Matrix4x4f perspective(float fov_y_radians, float aspect_ratio, float near_plane, float far_plane);
	static Matrix4x4f orthographic(float width, float height, float near_plane, float far_plane);
	static Matrix4x4f
	perspective_off_center(float left, float right, float bottom, float top, float near_plane, float far_plane);
	static Matrix4x4f
	orthographic_off_center(float left, float right, float bottom, float top, float near_plane, float far_plane);

	// The right-handed view matrix of a camera at `position` facing `target`: the inverse of its transform.
	static inline Matrix4x4f look_at(const Vector3f& position, const Vector3f& target, const Vector3f& up) {
		// view_look_to builds a +Z-forward frame; facing away from the target makes -Z the view direction.
		return from_rtm(
				rtm::view_look_to(position.to_rtm(), rtm::vector_sub(position.to_rtm(), target.to_rtm()), up.to_rtm())
		);
	}

	// `a * b` applies a, then b.
	RTM_FORCE_INLINE Matrix4x4f operator*(const Matrix4x4f& o) const { return from_rtm(rtm::matrix_mul(to_rtm(), o.to_rtm())); }
	RTM_FORCE_INLINE Matrix4x4f& operator*=(const Matrix4x4f& o) { return *this = *this * o; }

	[[nodiscard]] RTM_FORCE_INLINE Vector3f transform_point(const Vector3f& p) const {
		return Vector3f::from_rtm(rtm::matrix_mul_vector(rtm::vector_set(p.x, p.y, p.z, 1.0f), to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector3f transform_direction(const Vector3f& d) const {
		return Vector3f::from_rtm(rtm::matrix_mul_vector(rtm::vector_set(d.x, d.y, d.z, 0.0f), to_rtm()));
	}
	// The row vector `v * M`.
	RTM_FORCE_INLINE Vector4f operator*(const Vector4f& v) const {
		return Vector4f::from_rtm(rtm::matrix_mul_vector(v.to_rtm(), to_rtm()));
	}

	RTM_FORCE_INLINE bool operator==(const Matrix4x4f& o) const {
		return x_axis == o.x_axis && y_axis == o.y_axis && z_axis == o.z_axis && w_axis == o.w_axis;
	}
	RTM_FORCE_INLINE bool operator!=(const Matrix4x4f& o) const { return !(*this == o); }
	[[nodiscard]] inline bool is_near(const Matrix4x4f& o, float tolerance = 1.e-4f) const {
		return x_axis.is_near(o.x_axis, tolerance) && y_axis.is_near(o.y_axis, tolerance) &&
				z_axis.is_near(o.z_axis, tolerance) && w_axis.is_near(o.w_axis, tolerance);
	}

	[[nodiscard]] inline Matrix4x4f transposed() const { return from_rtm(rtm::matrix_transpose(to_rtm())); }
	[[nodiscard]] inline Matrix4x4f inverse() const { return from_rtm(rtm::matrix_inverse(to_rtm())); }
	[[nodiscard]] inline float determinant() const { return rtm::scalar_cast(rtm::matrix_determinant(to_rtm())); }

	// False when an axis has no length, which no rotation can be read from.
	bool decompose(Vector3f& out_translation, Quaternionf& out_rotation, Vector3f& out_scale) const;

	[[nodiscard]] RTM_FORCE_INLINE Vector3f translation() const { return { w_axis.x, w_axis.y, w_axis.z }; }
	RTM_FORCE_INLINE void set_translation(const Vector3f& t) { w_axis = { t.x, t.y, t.z, w_axis.w }; }

	RTM_FORCE_INLINE Vector4f& operator[](size_t i) { return (&x_axis)[i]; }
	RTM_FORCE_INLINE const Vector4f& operator[](size_t i) const { return (&x_axis)[i]; }

	static const Matrix4x4f identity;
};

static_assert(sizeof(Matrix4x4f) == 4 * sizeof(Vector4f), "Matrix4x4f must be exactly its four rows");
static_assert(std::is_standard_layout_v<Matrix4x4f>, "Matrix4x4f must stay standard-layout");

inline constexpr Matrix4x4f Matrix4x4f::identity {};

} //namespace feather

template <> struct std::formatter<feather::Matrix4x4f> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Matrix4x4f& m, std::format_context& ctx) {
		return std::format_to(ctx.out(), "[{} | {} | {} | {}]", m.x_axis, m.y_axis, m.z_axis, m.w_axis);
	}
};
