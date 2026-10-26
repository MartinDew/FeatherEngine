// C++ module wrapper for Realtime Math (`import rtm;`). RTM's headers are only ever included
// here, so the static SSE intrinsics they define exist in one module rather than one copy per
// importer's global module fragment, which clang rejects once two copies inline into one object.
module;

#include <rtm/camera_utilsf.h>
#include <rtm/matrix3x4f.h>
#include <rtm/matrix4x4f.h>
#include <rtm/quatd.h>
#include <rtm/quatf.h>
#include <rtm/scalard.h>
#include <rtm/scalarf.h>
#include <rtm/types.h>
#include <rtm/vector4d.h>
#include <rtm/vector4f.h>

export module rtm;

export namespace rtm {

using rtm::float2d;
using rtm::float2f;
using rtm::float3d;
using rtm::float3f;
using rtm::float4d;
using rtm::float4f;
using rtm::matrix3x4f;
using rtm::matrix4x4f;
using rtm::quatd;
using rtm::quatf;
using rtm::vector4d;
using rtm::vector4f;

using rtm::matrix_determinant;
using rtm::matrix_from_qvv;
using rtm::matrix_inverse;
using rtm::matrix_mul;
using rtm::matrix_mul_vector;
using rtm::matrix_transpose;
using rtm::quat_conjugate;
using rtm::quat_dot;
using rtm::quat_from_axis_angle;
using rtm::quat_from_matrix;
using rtm::quat_get_angle;
using rtm::quat_get_axis;
using rtm::quat_get_w;
using rtm::quat_get_x;
using rtm::quat_get_y;
using rtm::quat_get_z;
using rtm::quat_is_normalized;
using rtm::quat_length;
using rtm::quat_length_squared;
using rtm::quat_lerp;
using rtm::quat_mul;
using rtm::quat_mul_vector3;
using rtm::quat_near_equal;
using rtm::quat_normalize;
using rtm::quat_set;
using rtm::quat_slerp;
using rtm::scalar_cast;
using rtm::vector_abs;
using rtm::vector_add;
using rtm::vector_all_near_equal;
using rtm::vector_cross3;
using rtm::vector_div;
using rtm::vector_dot;
using rtm::vector_dot3;
using rtm::vector_get_w;
using rtm::vector_get_x;
using rtm::vector_get_y;
using rtm::vector_get_z;
using rtm::vector_length;
using rtm::vector_length3;
using rtm::vector_length_squared;
using rtm::vector_length_squared3;
using rtm::vector_lerp;
using rtm::vector_max;
using rtm::vector_min;
using rtm::vector_mul;
using rtm::vector_neg;
using rtm::vector_normalize;
using rtm::vector_normalize3;
using rtm::vector_set;
using rtm::vector_set_w;
using rtm::vector_sub;
using rtm::vector_zero;
using rtm::view_look_to;

} //namespace rtm
