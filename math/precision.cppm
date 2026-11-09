export module feather.math:precision;

export import :color;
export import :matrix;
export import :quaternion;
export import :vector2;
export import :vector3;
export import :vector4;

// The unsuffixed names follow the precision the build selected; the suffixed ones stay available
// to code that needs one precision.
export namespace feather {

#ifdef FEATHER_DOUBLE_PRECISION
using real_t = double;

using Vector2 = Vector2d;
using Vector3 = Vector3d;
using Vector4 = Vector4d;
using Quaternion = Quaterniond;
#else
using real_t = float;

using Vector2 = Vector2f;
using Vector3 = Vector3f;
using Vector4 = Vector4f;
using Quaternion = Quaternionf;
#endif

// Single precision either way: a color is a display quantity, and a matrix is what the GPU reads.
using Color = Colorf;
using Matrix = Matrix4x4f;

} //namespace feather
