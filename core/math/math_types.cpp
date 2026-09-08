#include "color.h"
#include "matrix.h"
#include "quaternion.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

// Out-of-line so each constant has one definition, and so a header include does
// not drag a static initializer into every translation unit that uses one.
namespace feather {

const Vector2f Vector2f::zero { 0.0f, 0.0f };
const Vector2f Vector2f::one { 1.0f, 1.0f };
const Vector2f Vector2f::right { 1.0f, 0.0f };
const Vector2f Vector2f::left { -1.0f, 0.0f };
const Vector2f Vector2f::up { 0.0f, 1.0f };
const Vector2f Vector2f::down { 0.0f, -1.0f };

const Vector2d Vector2d::zero { 0.0, 0.0 };
const Vector2d Vector2d::one { 1.0, 1.0 };
const Vector2d Vector2d::right { 1.0, 0.0 };
const Vector2d Vector2d::left { -1.0, 0.0 };
const Vector2d Vector2d::up { 0.0, 1.0 };
const Vector2d Vector2d::down { 0.0, -1.0 };

// Right-handed: +X right, +Y up, -Z forward.
const Vector3f Vector3f::zero { 0.0f, 0.0f, 0.0f };
const Vector3f Vector3f::one { 1.0f, 1.0f, 1.0f };
const Vector3f Vector3f::right { 1.0f, 0.0f, 0.0f };
const Vector3f Vector3f::left { -1.0f, 0.0f, 0.0f };
const Vector3f Vector3f::up { 0.0f, 1.0f, 0.0f };
const Vector3f Vector3f::down { 0.0f, -1.0f, 0.0f };
const Vector3f Vector3f::forward { 0.0f, 0.0f, -1.0f };
const Vector3f Vector3f::backward { 0.0f, 0.0f, 1.0f };

const Vector3d Vector3d::zero { 0.0, 0.0, 0.0 };
const Vector3d Vector3d::one { 1.0, 1.0, 1.0 };
const Vector3d Vector3d::right { 1.0, 0.0, 0.0 };
const Vector3d Vector3d::left { -1.0, 0.0, 0.0 };
const Vector3d Vector3d::up { 0.0, 1.0, 0.0 };
const Vector3d Vector3d::down { 0.0, -1.0, 0.0 };
const Vector3d Vector3d::forward { 0.0, 0.0, -1.0 };
const Vector3d Vector3d::backward { 0.0, 0.0, 1.0 };

const Vector4f Vector4f::zero { 0.0f, 0.0f, 0.0f, 0.0f };
const Vector4f Vector4f::one { 1.0f, 1.0f, 1.0f, 1.0f };

const Vector4d Vector4d::zero { 0.0, 0.0, 0.0, 0.0 };
const Vector4d Vector4d::one { 1.0, 1.0, 1.0, 1.0 };

const Quaternionf Quaternionf::identity { 0.0f, 0.0f, 0.0f, 1.0f };
const Quaterniond Quaterniond::identity { 0.0, 0.0, 0.0, 1.0 };

const Colorf Colorf::black { 0.0f, 0.0f, 0.0f, 1.0f };
const Colorf Colorf::white { 1.0f, 1.0f, 1.0f, 1.0f };
const Colorf Colorf::red { 1.0f, 0.0f, 0.0f, 1.0f };
const Colorf Colorf::green { 0.0f, 1.0f, 0.0f, 1.0f };
const Colorf Colorf::blue { 0.0f, 0.0f, 1.0f, 1.0f };
const Colorf Colorf::transparent { 0.0f, 0.0f, 0.0f, 0.0f };

const Matrix4x4f Matrix4x4f::identity {};

// Right-handed, depth in [0, 1]; the asymmetric counterparts of RTM's centred
// projections, which it does not provide.
Matrix4x4f Matrix4x4f::perspective_off_center(
		float left, float right, float bottom, float top, float near_plane, float far_plane
) {
	const float two_near = 2.0f * near_plane;
	const float inv_width = 1.0f / (right - left);
	const float inv_height = 1.0f / (top - bottom);
	const float range = far_plane / (near_plane - far_plane);

	return { { two_near * inv_width, 0.0f, 0.0f, 0.0f },
			 { 0.0f, two_near * inv_height, 0.0f, 0.0f },
			 { (left + right) * inv_width, (top + bottom) * inv_height, range, -1.0f },
			 { 0.0f, 0.0f, range * near_plane, 0.0f } };
}

Matrix4x4f Matrix4x4f::orthographic_off_center(
		float left, float right, float bottom, float top, float near_plane, float far_plane
) {
	const float inv_width = 1.0f / (right - left);
	const float inv_height = 1.0f / (top - bottom);
	const float inv_depth = 1.0f / (near_plane - far_plane);

	return { { 2.0f * inv_width, 0.0f, 0.0f, 0.0f },
			 { 0.0f, 2.0f * inv_height, 0.0f, 0.0f },
			 { 0.0f, 0.0f, inv_depth, 0.0f },
			 { -(left + right) * inv_width, -(top + bottom) * inv_height, near_plane * inv_depth, 1.0f } };
}

Quaternionf Quaternionf::from_matrix(const Matrix4x4f& m) {
	const rtm::matrix3x4f rotation = rtm::matrix_cast(m.to_rtm());
	return from_rtm(rtm::quat_from_matrix(rotation));
}

bool Matrix4x4f::decompose(Vector3f& out_translation, Quaternionf& out_rotation, Vector3f& out_scale) const {
	const Vector3f x { x_axis.x, x_axis.y, x_axis.z };
	const Vector3f y { y_axis.x, y_axis.y, y_axis.z };
	const Vector3f z { z_axis.x, z_axis.y, z_axis.z };

	out_scale = { x.length(), y.length(), z.length() };
	if (out_scale.x == 0.0f || out_scale.y == 0.0f || out_scale.z == 0.0f) {
		return false;
	}

	Matrix4x4f rotation_only = *this;
	rotation_only.x_axis = { x.x / out_scale.x, x.y / out_scale.x, x.z / out_scale.x, 0.0f };
	rotation_only.y_axis = { y.x / out_scale.y, y.y / out_scale.y, y.z / out_scale.y, 0.0f };
	rotation_only.z_axis = { z.x / out_scale.z, z.y / out_scale.z, z.z / out_scale.z, 0.0f };
	rotation_only.w_axis = { 0.0f, 0.0f, 0.0f, 1.0f };

	out_rotation = Quaternionf::from_matrix(rotation_only);
	out_translation = translation();
	return true;
}

} //namespace feather
