module feather.math;

import rtm;
import std;

namespace feather {

Matrix4x4f Matrix4x4f::perspective(float fov_y_radians, float aspect_ratio, float near_plane, float far_plane) {
	const float height = 1.0f / std::tan(fov_y_radians * 0.5f);
	const float width = height / aspect_ratio;
	const float range = far_plane / (near_plane - far_plane);

	return { { width, 0.0f, 0.0f, 0.0f },
			 { 0.0f, height, 0.0f, 0.0f },
			 { 0.0f, 0.0f, range, -1.0f },
			 { 0.0f, 0.0f, range * near_plane, 0.0f } };
}

Matrix4x4f Matrix4x4f::orthographic(float width, float height, float near_plane, float far_plane) {
	const float range = 1.0f / (near_plane - far_plane);

	return { { 2.0f / width, 0.0f, 0.0f, 0.0f },
			 { 0.0f, 2.0f / height, 0.0f, 0.0f },
			 { 0.0f, 0.0f, range, 0.0f },
			 { 0.0f, 0.0f, range * near_plane, 1.0f } };
}

Matrix4x4f Matrix4x4f::perspective_off_center(
		float left,
		float right,
		float bottom,
		float top,
		float near_plane,
		float far_plane
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
		float left,
		float right,
		float bottom,
		float top,
		float near_plane,
		float far_plane
) {
	const float inv_width = 1.0f / (right - left);
	const float inv_height = 1.0f / (top - bottom);
	const float range = 1.0f / (near_plane - far_plane);

	return { { 2.0f * inv_width, 0.0f, 0.0f, 0.0f },
			 { 0.0f, 2.0f * inv_height, 0.0f, 0.0f },
			 { 0.0f, 0.0f, range, 0.0f },
			 { -(left + right) * inv_width, -(top + bottom) * inv_height, range * near_plane, 1.0f } };
}

Quaternionf Quaternionf::from_matrix(const Matrix4x4f& m) {
	const rtm::matrix3x4f rotation { m.x_axis.to_rtm(), m.y_axis.to_rtm(), m.z_axis.to_rtm(), m.w_axis.to_rtm() };
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

	const Matrix4x4f rotation_only { { x.x / out_scale.x, x.y / out_scale.x, x.z / out_scale.x, 0.0f },
									 { y.x / out_scale.y, y.y / out_scale.y, y.z / out_scale.y, 0.0f },
									 { z.x / out_scale.z, z.y / out_scale.z, z.z / out_scale.z, 0.0f },
									 { 0.0f, 0.0f, 0.0f, 1.0f } };

	out_rotation = Quaternionf::from_matrix(rotation_only);
	out_translation = translation();
	return true;
}

Matrix4x4d Matrix4x4d::perspective(double fov_y_radians, double aspect_ratio, double near_plane, double far_plane) {
	const double height = 1.0 / std::tan(fov_y_radians * 0.5);
	const double width = height / aspect_ratio;
	const double range = far_plane / (near_plane - far_plane);

	return { { width, 0.0, 0.0, 0.0 },
			 { 0.0, height, 0.0, 0.0 },
			 { 0.0, 0.0, range, -1.0 },
			 { 0.0, 0.0, range * near_plane, 0.0 } };
}

Matrix4x4d Matrix4x4d::orthographic(double width, double height, double near_plane, double far_plane) {
	const double range = 1.0 / (near_plane - far_plane);

	return { { 2.0 / width, 0.0, 0.0, 0.0 },
			 { 0.0, 2.0 / height, 0.0, 0.0 },
			 { 0.0, 0.0, range, 0.0 },
			 { 0.0, 0.0, range * near_plane, 1.0 } };
}

Matrix4x4d Matrix4x4d::perspective_off_center(
		double left,
		double right,
		double bottom,
		double top,
		double near_plane,
		double far_plane
) {
	const double two_near = 2.0 * near_plane;
	const double inv_width = 1.0 / (right - left);
	const double inv_height = 1.0 / (top - bottom);
	const double range = far_plane / (near_plane - far_plane);

	return { { two_near * inv_width, 0.0, 0.0, 0.0 },
			 { 0.0, two_near * inv_height, 0.0, 0.0 },
			 { (left + right) * inv_width, (top + bottom) * inv_height, range, -1.0 },
			 { 0.0, 0.0, range * near_plane, 0.0 } };
}

Matrix4x4d Matrix4x4d::orthographic_off_center(
		double left,
		double right,
		double bottom,
		double top,
		double near_plane,
		double far_plane
) {
	const double inv_width = 1.0 / (right - left);
	const double inv_height = 1.0 / (top - bottom);
	const double range = 1.0 / (near_plane - far_plane);

	return { { 2.0 * inv_width, 0.0, 0.0, 0.0 },
			 { 0.0, 2.0 * inv_height, 0.0, 0.0 },
			 { 0.0, 0.0, range, 0.0 },
			 { -(left + right) * inv_width, -(top + bottom) * inv_height, range * near_plane, 1.0 } };
}

Quaterniond Quaterniond::from_matrix(const Matrix4x4d& m) {
	const rtm::matrix3x4d rotation { m.x_axis.to_rtm(), m.y_axis.to_rtm(), m.z_axis.to_rtm(), m.w_axis.to_rtm() };
	return from_rtm(rtm::quat_from_matrix(rotation));
}

bool Matrix4x4d::decompose(Vector3d& out_translation, Quaterniond& out_rotation, Vector3d& out_scale) const {
	const Vector3d x { x_axis.x, x_axis.y, x_axis.z };
	const Vector3d y { y_axis.x, y_axis.y, y_axis.z };
	const Vector3d z { z_axis.x, z_axis.y, z_axis.z };

	out_scale = { x.length(), y.length(), z.length() };
	if (out_scale.x == 0.0 || out_scale.y == 0.0 || out_scale.z == 0.0) {
		return false;
	}

	const Matrix4x4d rotation_only { { x.x / out_scale.x, x.y / out_scale.x, x.z / out_scale.x, 0.0 },
									 { y.x / out_scale.y, y.y / out_scale.y, y.z / out_scale.y, 0.0 },
									 { z.x / out_scale.z, z.y / out_scale.z, z.z / out_scale.z, 0.0 },
									 { 0.0, 0.0, 0.0, 1.0 } };

	out_rotation = Quaterniond::from_matrix(rotation_only);
	out_translation = translation();
	return true;
}

} //namespace feather
