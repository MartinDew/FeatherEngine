export module feather.reflection:builtin_types;

import std.compat;
import feather.framework;
import feather.math;
import :ftype;

// Descriptions of the types Variant stores natively. They live below reflection, so they can't carry their own FType
// the way an FSTRUCT does; ClassDB::register_builtin builds their ClassInfo from these. The names are the precision
// aliases, whichever type the build selects.
namespace feather {

namespace builtin_detail {

template <auto Member>
using Field = PropertyDesc<Member, Member>;

} // namespace builtin_detail

#define FEATHER_BUILTIN_FTYPE(type_name, ...)                                                                          \
	template <>                                                                                                        \
	struct FType<type_name> {                                                                                          \
		using type = type_name;                                                                                        \
		using parent = void;                                                                                           \
		static constexpr StaticString name = StaticString(#type_name);                                                          \
		static constexpr StaticString parent_name = ""_ss;                                                             \
		static constexpr auto properties = std::tuple { __VA_ARGS__ };                                                 \
		static constexpr auto methods = std::tuple {};                                                                 \
	};

#define FEATHER_FIELD(type_name, field) builtin_detail::Field<&type_name::field> { StaticString(#field) }

FEATHER_BUILTIN_FTYPE(Vector2, FEATHER_FIELD(Vector2, x), FEATHER_FIELD(Vector2, y))
FEATHER_BUILTIN_FTYPE(Vector3, FEATHER_FIELD(Vector3, x), FEATHER_FIELD(Vector3, y), FEATHER_FIELD(Vector3, z))
FEATHER_BUILTIN_FTYPE(
		Vector4, FEATHER_FIELD(Vector4, x), FEATHER_FIELD(Vector4, y), FEATHER_FIELD(Vector4, z), FEATHER_FIELD(Vector4, w)
)
FEATHER_BUILTIN_FTYPE(
		Quaternion,
		FEATHER_FIELD(Quaternion, x),
		FEATHER_FIELD(Quaternion, y),
		FEATHER_FIELD(Quaternion, z),
		FEATHER_FIELD(Quaternion, w)
)
FEATHER_BUILTIN_FTYPE(Color, FEATHER_FIELD(Color, r), FEATHER_FIELD(Color, g), FEATHER_FIELD(Color, b), FEATHER_FIELD(Color, a))
FEATHER_BUILTIN_FTYPE(
		Matrix,
		FEATHER_FIELD(Matrix, x_axis),
		FEATHER_FIELD(Matrix, y_axis),
		FEATHER_FIELD(Matrix, z_axis),
		FEATHER_FIELD(Matrix, w_axis)
)
FEATHER_BUILTIN_FTYPE(
		Transform, FEATHER_FIELD(Transform, position), FEATHER_FIELD(Transform, rotation), FEATHER_FIELD(Transform, scale)
)
FEATHER_BUILTIN_FTYPE(
		Vertex, FEATHER_FIELD(Vertex, position), FEATHER_FIELD(Vertex, normal), FEATHER_FIELD(Vertex, uv)
)

#undef FEATHER_FIELD
#undef FEATHER_BUILTIN_FTYPE

} // namespace feather
