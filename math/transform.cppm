module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "transform.gen.h"
#endif

export module feather.math:transform;

import std;
import feather.framework;
import :math_defs;

export namespace feather {

struct FEATHER_MATH_API Transform {
	FSTRUCT(builtin);

	// Feather uses right-handed coordinates
	// up = Y
	// right = X
	// forward = -Z
	[[get, set]] Vector3 position = Vector3::zero; // X, Y, Z
	[[get, set]] Quaternion rotation = Quaternion::identity; // Quaternion
	[[get, set]] Vector3 scale = Vector3::one; // X, Y, Z

	Transform() = default;
	explicit Transform(const Matrix& transformationMat);
	Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);

	[[method]]
	static Transform construct_from_matrices_and_scale(const Matrix& mat1, const Matrix& mat2, Vector3 desiredScale);

	[[method]]
	static Transform multiply(const Transform& a, const Transform& b);

	[[method]]
	Transform get_relative_transform(const Transform& other) const;
	[[method]]
	Transform get_relative_transform_reverse(const Transform& other) const;
	[[method]]
	void set_to_relative_transform(const Transform& parent);

	inline void translate(const Vector3& translation) noexcept { position += translation; }
	inline void apply_scaling(const Vector3& scaling) noexcept { scale *= scaling; }
	[[method]]
	Vector3 get_forward_vector() const noexcept;
	[[method]]
	Vector3 get_up_vector() const noexcept;
	[[method]]
	Vector3 get_right_vector() const noexcept;

	[[method]]
	Transform inverse() const noexcept;

	[[method]]
	void look_at(const Vector3& eye, const Vector3& target, const Vector3& up);
	[[method]]
	void look_at(const Vector3& target, const Vector3& up);

	[[method]]
	void look_towards(const Vector3& direction, const Vector3& up);
	[[method]]
	void rotate(const int dx, const int dy);
	[[method]]
	void rotate(const Quaternion& rotation) noexcept;
	[[method]]
	void rotate_to(const Vector3& eulerAngles);

	[[method]]
	bool is_rotation_normalized() const;

	// Better when the three are needed { up, right, forward }
	std::tuple<Vector3, Vector3, Vector3> get_axis() const noexcept;

	[[method]]
	static Transform create_look_at(const Vector3& eye, const Vector3& target, const Vector3& up);

	bool operator==(const Transform&) const;
	inline Transform operator*(const Transform& other) const { return multiply(*this, other); }

	inline Transform& operator*=(const Transform& other) {
		*this = multiply(*this, other);
		return *this;
	}

	// Matrices
	[[method]]
	Matrix to_matrix_with_scale() const noexcept;
	[[method]]
	Matrix to_matrix_no_scale() const noexcept;
	[[method]]
	static Transform multiply_using_matrix_with_scale(const Transform& a, const Transform& b);
	[[method]]
	static Transform get_relative_transform_using_matrix_with_scale(const Transform& base, const Transform& relative);
};

Transform make_transform_screen_space_sized_billboard(
		Transform objectTransform,
		Vector3 cameraPosition,
		float fovDeg,
		Vector2 minScreenSpaceSize,
		Vector2 screenSize
);
FEATHER_MATH_API Transform make_transform_billboard(Transform objectTransform, Vector3 cameraPosition);

} //namespace feather
