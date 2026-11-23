module;

#include <framework/reflection_macros.h>

#include <rtm/impl/compiler_utils.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "matrix.gen.h"
#endif

export module feather.math:matrix;

import rtm;
import std.compat;
import feather.framework;
import :quaternion;
import :vector3;
import :vector4;

export namespace feather {

struct Matrix4x4d;

// A 4x4 transformation matrix stored as four rows, for row vectors (`v * M`): rows 0-2 are the
// basis axes and row 3 the translation, the layout RTM and the GPU uploads both use.
struct Matrix4x4f {
	FSTRUCT(builtin(Matrix));

	[[get, set]] Vector4f x_axis = { 1.0f, 0.0f, 0.0f, 0.0f };
	[[get, set]] Vector4f y_axis = { 0.0f, 1.0f, 0.0f, 0.0f };
	[[get, set]] Vector4f z_axis = { 0.0f, 0.0f, 1.0f, 0.0f };
	[[get, set]] Vector4f w_axis = { 0.0f, 0.0f, 0.0f, 1.0f };

	constexpr Matrix4x4f() = default;
	// Narrows each element, as for a GPU upload.
	constexpr Matrix4x4f(const Matrix4x4d& m);
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
	static inline Matrix4x4f
	from_transform(const Vector3f& translation, const Quaternionf& rotation, const Vector3f& scale) {
		return from_rtm(rtm::matrix_from_qvv(rotation.to_rtm(), translation.to_rtm(), scale.to_rtm()));
	}

	// Right-handed, looking down -Z, depth mapped to [0, 1]. RTM's own camera helpers are
	// left-handed, so these are written out here.
	FEATHER_MATH_API static Matrix4x4f
	perspective(float fov_y_radians, float aspect_ratio, float near_plane, float far_plane);
	FEATHER_MATH_API static Matrix4x4f orthographic(float width, float height, float near_plane, float far_plane);
	FEATHER_MATH_API static Matrix4x4f
	perspective_off_center(float left, float right, float bottom, float top, float near_plane, float far_plane);
	FEATHER_MATH_API static Matrix4x4f
	orthographic_off_center(float left, float right, float bottom, float top, float near_plane, float far_plane);

	// The right-handed view matrix of a camera at `position` facing `target`: the inverse of its transform.
	static inline Matrix4x4f look_at(const Vector3f& position, const Vector3f& target, const Vector3f& up) {
		// view_look_to builds a +Z-forward frame; facing away from the target makes -Z the view direction.
		return from_rtm(
				rtm::view_look_to(position.to_rtm(), rtm::vector_sub(position.to_rtm(), target.to_rtm()), up.to_rtm())
		);
	}

	// `a * b` applies a, then b.
	RTM_FORCE_INLINE Matrix4x4f operator*(const Matrix4x4f& o) const {
		return from_rtm(rtm::matrix_mul(to_rtm(), o.to_rtm()));
	}
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
	FEATHER_MATH_API bool decompose(Vector3f& out_translation, Quaternionf& out_rotation, Vector3f& out_scale) const;

	[[nodiscard]] RTM_FORCE_INLINE Vector3f translation() const { return { w_axis.x, w_axis.y, w_axis.z }; }
	RTM_FORCE_INLINE void set_translation(const Vector3f& t) { w_axis = { t.x, t.y, t.z, w_axis.w }; }

	RTM_FORCE_INLINE Vector4f& operator[](size_t i) { return (&x_axis)[i]; }
	RTM_FORCE_INLINE const Vector4f& operator[](size_t i) const { return (&x_axis)[i]; }

	static const Matrix4x4f identity;
};

static_assert(sizeof(Matrix4x4f) == 4 * sizeof(Vector4f), "Matrix4x4f must be exactly its four rows");
static_assert(std::is_standard_layout_v<Matrix4x4f>, "Matrix4x4f must stay standard-layout");

inline constexpr Matrix4x4f Matrix4x4f::identity {};

// The double-precision counterpart of Matrix4x4f, with the same layout and conventions.
struct Matrix4x4d {
	FSTRUCT(builtin(Matrix));

	[[get, set]] Vector4d x_axis = { 1.0, 0.0, 0.0, 0.0 };
	[[get, set]] Vector4d y_axis = { 0.0, 1.0, 0.0, 0.0 };
	[[get, set]] Vector4d z_axis = { 0.0, 0.0, 1.0, 0.0 };
	[[get, set]] Vector4d w_axis = { 0.0, 0.0, 0.0, 1.0 };

	constexpr Matrix4x4d() = default;
	constexpr Matrix4x4d(const Matrix4x4f& m)
			: x_axis(m.x_axis)
			, y_axis(m.y_axis)
			, z_axis(m.z_axis)
			, w_axis(m.w_axis) {}
	constexpr Matrix4x4d(const Vector4d& x, const Vector4d& y, const Vector4d& z, const Vector4d& w)
			: x_axis(x)
			, y_axis(y)
			, z_axis(z)
			, w_axis(w) {}

	[[nodiscard]] RTM_FORCE_INLINE rtm::matrix4x4d to_rtm() const {
		return rtm::matrix4x4d { x_axis.to_rtm(), y_axis.to_rtm(), z_axis.to_rtm(), w_axis.to_rtm() };
	}
	static RTM_FORCE_INLINE Matrix4x4d from_rtm(const rtm::matrix4x4d& m) {
		return { Vector4d::from_rtm(m.x_axis),
				 Vector4d::from_rtm(m.y_axis),
				 Vector4d::from_rtm(m.z_axis),
				 Vector4d::from_rtm(m.w_axis) };
	}
	// An affine 3x4 leaves its W lanes undefined; they are set to the affine row's 0, 0, 0, 1.
	static RTM_FORCE_INLINE Matrix4x4d from_rtm(const rtm::matrix3x4d& m) {
		return { Vector4d::from_rtm(rtm::vector_set_w(m.x_axis, 0.0)),
				 Vector4d::from_rtm(rtm::vector_set_w(m.y_axis, 0.0)),
				 Vector4d::from_rtm(rtm::vector_set_w(m.z_axis, 0.0)),
				 Vector4d::from_rtm(rtm::vector_set_w(m.w_axis, 1.0)) };
	}

	static inline Matrix4x4d from_translation(const Vector3d& t) {
		Matrix4x4d m;
		m.w_axis = { t.x, t.y, t.z, 1.0 };
		return m;
	}
	static inline Matrix4x4d from_rotation(const Quaterniond& r) {
		return from_rtm(rtm::matrix_from_qvv(r.to_rtm(), rtm::vector_zero(), rtm::vector_set(1.0)));
	}
	static inline Matrix4x4d from_scale(const Vector3d& s) {
		Matrix4x4d m;
		m.x_axis = { s.x, 0.0, 0.0, 0.0 };
		m.y_axis = { 0.0, s.y, 0.0, 0.0 };
		m.z_axis = { 0.0, 0.0, s.z, 0.0 };
		return m;
	}
	// Scale, then rotate, then translate.
	static inline Matrix4x4d
	from_transform(const Vector3d& translation, const Quaterniond& rotation, const Vector3d& scale) {
		return from_rtm(rtm::matrix_from_qvv(rotation.to_rtm(), translation.to_rtm(), scale.to_rtm()));
	}

	// Right-handed, looking down -Z, depth mapped to [0, 1]. RTM's own camera helpers are
	// left-handed, so these are written out here.
	FEATHER_MATH_API static Matrix4x4d
	perspective(double fov_y_radians, double aspect_ratio, double near_plane, double far_plane);
	FEATHER_MATH_API static Matrix4x4d orthographic(double width, double height, double near_plane, double far_plane);
	FEATHER_MATH_API static Matrix4x4d
	perspective_off_center(double left, double right, double bottom, double top, double near_plane, double far_plane);
	FEATHER_MATH_API static Matrix4x4d
	orthographic_off_center(double left, double right, double bottom, double top, double near_plane, double far_plane);

	// The right-handed view matrix of a camera at `position` facing `target`: the inverse of its transform.
	static inline Matrix4x4d look_at(const Vector3d& position, const Vector3d& target, const Vector3d& up) {
		// view_look_to builds a +Z-forward frame; facing away from the target makes -Z the view direction.
		return from_rtm(
				rtm::view_look_to(position.to_rtm(), rtm::vector_sub(position.to_rtm(), target.to_rtm()), up.to_rtm())
		);
	}

	// `a * b` applies a, then b.
	RTM_FORCE_INLINE Matrix4x4d operator*(const Matrix4x4d& o) const {
		return from_rtm(rtm::matrix_mul(to_rtm(), o.to_rtm()));
	}
	RTM_FORCE_INLINE Matrix4x4d& operator*=(const Matrix4x4d& o) { return *this = *this * o; }

	[[nodiscard]] RTM_FORCE_INLINE Vector3d transform_point(const Vector3d& p) const {
		return Vector3d::from_rtm(rtm::matrix_mul_vector(rtm::vector_set(p.x, p.y, p.z, 1.0), to_rtm()));
	}
	[[nodiscard]] RTM_FORCE_INLINE Vector3d transform_direction(const Vector3d& d) const {
		return Vector3d::from_rtm(rtm::matrix_mul_vector(rtm::vector_set(d.x, d.y, d.z, 0.0), to_rtm()));
	}
	// The row vector `v * M`.
	RTM_FORCE_INLINE Vector4d operator*(const Vector4d& v) const {
		return Vector4d::from_rtm(rtm::matrix_mul_vector(v.to_rtm(), to_rtm()));
	}

	RTM_FORCE_INLINE bool operator==(const Matrix4x4d& o) const {
		return x_axis == o.x_axis && y_axis == o.y_axis && z_axis == o.z_axis && w_axis == o.w_axis;
	}
	RTM_FORCE_INLINE bool operator!=(const Matrix4x4d& o) const { return !(*this == o); }
	[[nodiscard]] inline bool is_near(const Matrix4x4d& o, double tolerance = 1.e-8) const {
		return x_axis.is_near(o.x_axis, tolerance) && y_axis.is_near(o.y_axis, tolerance) &&
				z_axis.is_near(o.z_axis, tolerance) && w_axis.is_near(o.w_axis, tolerance);
	}

	[[nodiscard]] inline Matrix4x4d transposed() const { return from_rtm(rtm::matrix_transpose(to_rtm())); }
	[[nodiscard]] inline Matrix4x4d inverse() const { return from_rtm(rtm::matrix_inverse(to_rtm())); }
	[[nodiscard]] inline double determinant() const { return rtm::scalar_cast(rtm::matrix_determinant(to_rtm())); }

	// False when an axis has no length, which no rotation can be read from.
	FEATHER_MATH_API bool decompose(Vector3d& out_translation, Quaterniond& out_rotation, Vector3d& out_scale) const;

	[[nodiscard]] RTM_FORCE_INLINE Vector3d translation() const { return { w_axis.x, w_axis.y, w_axis.z }; }
	RTM_FORCE_INLINE void set_translation(const Vector3d& t) { w_axis = { t.x, t.y, t.z, w_axis.w }; }

	RTM_FORCE_INLINE Vector4d& operator[](size_t i) { return (&x_axis)[i]; }
	RTM_FORCE_INLINE const Vector4d& operator[](size_t i) const { return (&x_axis)[i]; }

	static const Matrix4x4d identity;
};

static_assert(sizeof(Matrix4x4d) == 4 * sizeof(Vector4d), "Matrix4x4d must be exactly its four rows");
static_assert(std::is_standard_layout_v<Matrix4x4d>, "Matrix4x4d must stay standard-layout");

inline constexpr Matrix4x4d Matrix4x4d::identity {};

constexpr Matrix4x4f::Matrix4x4f(const Matrix4x4d& m)
		: x_axis(m.x_axis)
		, y_axis(m.y_axis)
		, z_axis(m.z_axis)
		, w_axis(m.w_axis) {
}

} //namespace feather

template <> struct std::formatter<feather::Matrix4x4f> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Matrix4x4f& m, std::format_context& ctx) {
		return std::format_to(ctx.out(), "[{} | {} | {} | {}]", m.x_axis, m.y_axis, m.z_axis, m.w_axis);
	}
};

template <> struct std::formatter<feather::Matrix4x4d> {
	static constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }

	static inline auto format(const feather::Matrix4x4d& m, std::format_context& ctx) {
		return std::format_to(ctx.out(), "[{} | {} | {} | {}]", m.x_axis, m.y_axis, m.z_axis, m.w_axis);
	}
};
