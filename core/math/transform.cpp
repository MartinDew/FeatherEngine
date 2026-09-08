#include "transform.h"

#include <cassert>

#include <numbers>

namespace {

using feather::Vector3;

bool HasNegativeScale(const Vector3& scale, const Vector3& otherScale) {
	return scale.any_component_less_equal(0.0f) || otherScale.any_component_less_equal(0.0f);
}

template <class T>
	requires std::is_floating_point_v<T>
Vector3 GetReciprocalSafe(const Vector3& inVec, T tolerance) {
	return inVec.reciprocal_safe(static_cast<float>(tolerance));
}

} //namespace

namespace feather {

Transform Transform::multiply(const Transform& a, const Transform& b) {
#ifdef SC_DEV_VERSION
	assert(a.is_rotation_normalized());
	assert(b.is_rotation_normalized());
#endif

	// check if negative scale
	if (HasNegativeScale(a.scale, b.scale)) {
		return multiply_using_matrix_with_scale(a, b);
	}

	// else
	const Quaternion quatA = a.rotation;
	const Quaternion quatB = b.rotation;
	const Vector3 translateA = a.position;
	const Vector3 translateB = b.position;
	const Vector3 scaleA = a.scale;
	const Vector3 scaleB = b.scale;

	Transform out;
	out.rotation = quatB * quatA;

	const Vector3 scaledTransA = translateA * scaleB;
	const Vector3 rotatedTranslate = quatB * scaledTransA;
	out.position = rotatedTranslate + translateB;
	out.scale = scaleA * scaleB;

	return out;
}

Transform Transform::multiply_using_matrix_with_scale(const Transform& a, const Transform& b) {
	return construct_from_matrices_and_scale(a.to_matrix_with_scale(), b.to_matrix_with_scale(), a.scale * b.scale);
}

Matrix Transform::to_matrix_with_scale() const noexcept {
	return Matrix::from_transform(position, rotation, scale);
}

Matrix Transform::to_matrix_no_scale() const noexcept {
	return Matrix::from_transform(position, rotation, Vector3::one);
}

Vector3 Transform::get_forward_vector() const noexcept {
	return rotation * Vector3::forward;
}

Vector3 Transform::get_up_vector() const noexcept {
	assert(is_rotation_normalized());
	return rotation * Vector3::up;
}

Vector3 Transform::get_right_vector() const noexcept {
	return rotation * Vector3::right;
}

Transform Transform::inverse() const noexcept {
	const Quaternion invRot = rotation.inverse();
	const Vector3 invScale = scale.reciprocal_safe();

	// invert the translation
	const Vector3 scaledTranslation = invScale * position;
	const Vector3 invTranslation = -(invRot * scaledTranslation);

	return { invTranslation, invRot, invScale };
}

void Transform::look_at(const Vector3& eye, const Vector3& target, const Vector3& up) {
	position = eye;
	look_at(target, up);
}

void Transform::look_at(const Vector3& target, const Vector3& up) {
	// look_at builds the view matrix, which is the inverse of the orientation
	// this transform should take.
	const Matrix mat = Matrix::look_at(position, target, up);
	rotation = Quaternion::from_matrix(mat).inverse();
}

void Transform::look_towards(const Vector3& direction, const Vector3& up) {
	look_at(position + direction, up);
}

void Transform::rotate(const int dx, const int dy) {
	const Quaternion rotx = Quaternion::from_axis_angle(Vector3::up, -dx / 1000.0f);
	const Quaternion roty = Quaternion::from_axis_angle(get_right_vector(), -dy / 1000.0f);
	rotation *= roty * rotx;
	rotation.normalize();
}

void Transform::rotate(const Quaternion& rotation) noexcept {
	this->rotation *= rotation.normalized();
}

void Transform::rotate_to(const Vector3& eulerAngles) {
	// Convert Euler angles (in radians)
	auto targetRotation = Quaternion::from_euler(eulerAngles.y, eulerAngles.x, eulerAngles.z);

	// Normalize current and target rotations
	rotation.normalize();
	targetRotation.normalize();

	// Use spherical linear interpolation to prevent gimbal lock
	rotation = rotation.slerp(targetRotation, 1.0f);
}

bool Transform::is_rotation_normalized() const {
	return std::abs(1.0f - rotation.length_squared()) <= quaternion_normalize_threshhold;
}

std::tuple<Vector3, Vector3, Vector3> Transform::get_axis() const noexcept {
	return { get_up_vector(), get_right_vector(), get_forward_vector() };
}

Transform Transform::create_look_at(const Vector3& eye, const Vector3& target, const Vector3& up) {
	Transform t;
	t.look_at(eye, target, up);
	return t;
}

Transform::Transform(const Matrix& transformationMat) {
	[[maybe_unused]] const bool decomposed = transformationMat.decompose(position, rotation, scale);
	assert(decomposed);
}

Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
		: position(position)
		, rotation(rotation)
		, scale(scale) {
}

Transform Transform::construct_from_matrices_and_scale(const Matrix& mat1, const Matrix& mat2, Vector3 desiredScale) {
	using namespace math::matrices;
	Transform result;
	Matrix mat = mat1 * mat2;
	math::matrices::remove_scaling(mat);

	Vector3 signedScale = desiredScale;

	set_axis(mat, 0, signedScale.x * math::matrices::get_axis(mat, math::matrices::Axis::X));
	set_axis(mat, 1, signedScale.y * math::matrices::get_axis(mat, math::matrices::Axis::Y));
	set_axis(mat, 2, signedScale.z * math::matrices::get_axis(mat, math::matrices::Axis::Z));

	Quaternion rot = Quaternion::from_matrix(mat);
	rot.normalize();

	result.scale = desiredScale;
	result.rotation = rot;

	result.position = get_origin(mat);
	return result;
}

Transform make_transform_screen_space_sized_billboard(Transform objectTransform,
													  Vector3 cameraPosition,
													  float fovDeg,
													  Vector2 minScreenSpaceSize,
													  Vector2 screenSize) {
	Vector3 pos = objectTransform.position;
	objectTransform.rotation = Quaternion::from_matrix(
			Matrix::look_at(pos, cameraPosition, objectTransform.get_up_vector()).inverse());
	float distanceToObject = cameraPosition.distance(pos);
	const float maxConstantSizeDistance = 200.0f; // After this distance, object will scale with perspective

	float effectiveDistance = distanceToObject;
	if (distanceToObject > maxConstantSizeDistance) {
		// Scale quadratically beyond max distance to match perspective scaling
		float ratio = maxConstantSizeDistance / distanceToObject;
		effectiveDistance = maxConstantSizeDistance * ratio;
	}

	float screenHeightInWorldUnits =
			2.0f * tanf(fovDeg * 0.5f * (std::numbers::pi_v<float> / 180.0f)) * effectiveDistance;
	Vector2 desiredWorldSize = (minScreenSpaceSize / screenSize) * screenHeightInWorldUnits;

	Vector2 scaleFactors;
	scaleFactors.x = desiredWorldSize.x / objectTransform.scale.x;
	scaleFactors.y = desiredWorldSize.y / objectTransform.scale.y;

	float scaleFactor = std::max(scaleFactors.x, scaleFactors.y);
	objectTransform.scale.x *= scaleFactor;
	objectTransform.scale.y *= scaleFactor;

	return objectTransform;
}

Transform make_transform_billboard(Transform objectTransform, Vector3 cameraPosition) {
	Vector3 pos = objectTransform.position;
	objectTransform.rotation = Quaternion::from_matrix(
			Matrix::look_at(pos, cameraPosition, objectTransform.get_up_vector()).inverse());
	return objectTransform;
}

bool Transform::operator==(const Transform& other) const {
	return std::tie(position, scale, rotation) == std::tie(other.position, other.scale, other.rotation);
}

Transform Transform::get_relative_transform(const Transform& other) const {
	Transform result;

	if (!other.is_rotation_normalized())
		return Transform {};

	if (HasNegativeScale(scale, other.scale)) {
		return multiply_using_matrix_with_scale(*this, other);
	}

	Vector3 safeScale = GetReciprocalSafe(other.scale, small_number);

	Vector3 desiredScale = scale * safeScale;

	Vector3 translation = position - other.position;

	Quaternion invRot = other.rotation.inverse();

	Vector3 vr = invRot * translation;
	Vector3 vtranslation = vr * safeScale;

	Quaternion vrotation = rotation * invRot;

	result.position = vtranslation;
	result.rotation = vrotation;
	result.scale = desiredScale;

	return result;
}

Transform Transform::get_relative_transform_reverse(const Transform& other) const {
	return other.get_relative_transform({ *this });
}

void Transform::set_to_relative_transform(const Transform& parent) {
	*this = get_relative_transform(parent);
}

Transform Transform::get_relative_transform_using_matrix_with_scale(const Transform& base, const Transform& relative) {
	Matrix A = base.to_matrix_with_scale();
	Matrix B = relative.to_matrix_with_scale();

	Vector3 scale = relative.scale.reciprocal_safe();
	Vector3 desiredScale = base.scale * scale;

	return construct_from_matrices_and_scale(A, B.inverse(), desiredScale);
}

} //namespace feather