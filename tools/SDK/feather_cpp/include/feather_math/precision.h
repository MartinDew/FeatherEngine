#pragma once

#include "color.h"
#include "matrix.h"
#include "quaternion.h"
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"

namespace feather {

// The precision the engine computes in, and the unsuffixed names that follow
// from it: engine and game code says Vector3 and gets whichever variant the
// build selected.
//
// FEATHER_DOUBLE_PRECISION is not wired to a build option yet. The double
// variants exist and compile, so turning it on is the only step left.
//
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

// Single precision either way: a color is a display quantity (see color.h), and
// a matrix holds RTM's own float rows (see matrix.h).
using Color = Colorf;
using Matrix = Matrix4x4f;

// The precision-suffixed names stay available either way, so code that genuinely
// needs one precision can say so.

} //namespace feather
