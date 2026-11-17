module feather.reflection;

import std;

namespace feather {

ObjectRef Variant::_object_ref() const {
	if (auto* ref = std::get_if<ObjectRef>(&_data)) {
		return *ref;
	}
	return {};
}

bool Variant::operator==(const Variant& other) const {
	// Types must match
	if (_type != other._type) {
		return false;
	}

	// Use std::visit to compare the values
	return std::visit(
			[](const auto& lhs, const auto& rhs) -> bool {
				using LhsType = std::decay_t<decltype(lhs)>;
				using RhsType = std::decay_t<decltype(rhs)>;

				// If types don't match at compile time, return false
				if constexpr (!std::is_same_v<LhsType, RhsType>) {
					return false;
				}
				// monostate (NIL) is always equal
				else if constexpr (std::is_same_v<LhsType, std::monostate>) {
					return true;
				}
				// For all other types, use their operator==
				else {
					return lhs == rhs;
				}
			},
			_data,
			other._data
	);
}

std::string_view Variant::get_type_name(VariantType type) {
	switch (type) {
	case VariantType::NIL:
		return "Nil";
	case VariantType::BOOL:
		return "bool";
	case VariantType::INT:
		return "int";
	case VariantType::FLOAT:
		return "float";
	case VariantType::VECTOR3:
		return "Vector3";
	case VariantType::VECTOR2:
		return "Vector2";
	case VariantType::VERTEX:
		return "Vertex";
	case VariantType::COLOR:
		return "Color";
	case VariantType::VECTOR4:
		return "Vector4";
	case VariantType::QUATERNION:
		return "Quaternion";
	case VariantType::MATRIX:
		return "Matrix";
	case VariantType::TRANSFORM:
		return "Transform";
	case VariantType::RID:
		return "RID";
	case VariantType::STRING:
		return "String";
	case VariantType::ARRAY:
		return "Array";
	case VariantType::PATH:
		return "Path";
	case VariantType::OBJECT:
		return "Object";
	case VariantType::INVALID:
		break;
	}
	return "Invalid";
}

bool Variant::booleanize() const {
	return std::visit(
			[](const auto& value) -> bool {
				using T = std::decay_t<decltype(value)>;
				if constexpr (std::is_same_v<T, std::monostate>) {
					return false;
				}
				else if constexpr (std::is_same_v<T, bool>) {
					return value;
				}
				else if constexpr (std::is_arithmetic_v<T>) {
					return value != T {};
				}
				else if constexpr (
						std::is_same_v<T, std::string> || std::is_same_v<T, Path> || std::is_same_v<T, VariantArray>
				)
				{
					return !value.empty();
				}
				else if constexpr (std::is_same_v<T, ObjectRef>) {
					return static_cast<bool>(value);
				}
				else if constexpr (std::is_same_v<T, RID>) {
					return value != RID {};
				}
				else {
					// Math values: anything but the default (zero vector, identity quaternion/matrix/transform).
					return !(value == T {});
				}
			},
			_data
	);
}

std::string Variant::to_string() const {
	switch (_type) {
	case VariantType::NIL:
		return "nil";
	case VariantType::BOOL:
		return std::get<bool>(_data) ? "true" : "false";
	case VariantType::INT:
		return std::to_string(std::get<int64_t>(_data));
	case VariantType::FLOAT:
		return std::format("{:f}", std::get<real_t>(_data));
	case VariantType::STRING:
		return std::get<std::string>(_data);
	case VariantType::VECTOR2: {
		auto& v = std::get<Vector2>(_data);
		return std::format("({:f}, {:f})", v.x, v.y);
	}
	case VariantType::VECTOR3: {
		auto& v = std::get<Vector3>(_data);
		return std::format("({:f}, {:f}, {:f})", v.x, v.y, v.z);
	}
	case VariantType::COLOR: {
		auto& c = std::get<Color>(_data);
		return std::format("rgba({:f}, {:f}, {:f}, {:f})", c.r, c.g, c.b, c.a);
	}
	case VariantType::VECTOR4: {
		auto& v = std::get<Vector4>(_data);
		return std::format("({:f}, {:f}, {:f}, {:f})", v.x, v.y, v.z, v.w);
	}
	case VariantType::QUATERNION: {
		auto& q = std::get<Quaternion>(_data);
		return std::format("({:f}, {:f}, {:f}, {:f})", q.x, q.y, q.z, q.w);
	}
	case VariantType::MATRIX:
		return "[Matrix]";
	case VariantType::TRANSFORM:
		return "[Transform]";
	case VariantType::VERTEX:
		return "[Vertex]"; // expand if Vertex gains a meaningful string form
	case VariantType::PATH:
		return std::get<Path>(_data).string(); // assumes Path has to_string()
	case VariantType::RID:
		return "RID(" + std::to_string(std::get<RID>(_data).id) + ")"; // assumes RID has .id
	case VariantType::ARRAY:
		return "[Array]";
	case VariantType::OBJECT:
		return "[Object] : " + get_name();
	case VariantType::INVALID:
		return "[Invalid]";
	}
	return "[Unknown]";
}

std::string Variant::get_name() const {
	if (_type != VariantType::OBJECT) {
		return to_string();
	}
	ObjectRef ref = _object_ref();
	if (!ref.get_class_info()) {
		return "[Object: no class info]";
	}
	return std::string(ref.get_class_name());
}

namespace {

// Walks the object's class hierarchy; see ClassInfo::walk.
template <class TVisit>
bool walk_hierarchy(ObjectRef ref, TVisit&& visit) {
	const ClassInfo* info = ref.get_class_info();
	return info && info->walk(ref.get(), std::forward<TVisit>(visit));
}

Variant get_property(ObjectRef ref, std::string_view key, bool enforce_public) {
	Variant result;
	walk_hierarchy(ref, [&](const ClassInfo& info, void* object) {
		for (auto& property : info.properties) {
			if (property.name == key) {
				// Script-facing access: only public getters are reachable.
				if (property.getter && (!enforce_public || property.getter_access == AccessLevel::Public)) {
					result = property.getter(object);
				}
				return true;
			}
		}
		return false;
	});
	return result;
}

void set_property(ObjectRef ref, std::string_view key, const Variant& value, bool enforce_public) {
	walk_hierarchy(ref, [&](const ClassInfo& info, void* object) {
		for (auto& property : info.properties) {
			if (property.name == key) {
				// Script-facing access: only public setters are writable.
				if (property.setter && (!enforce_public || property.setter_access == AccessLevel::Public)) {
					property.setter(object, value);
				}
				return true;
			}
		}
		return false;
	});
}

} // namespace

Variant Variant::get(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	return get_property(_object_ref(), key, true);
}

Variant Variant::get_internal(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	return get_property(_object_ref(), key, false);
}

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	set_property(_object_ref(), key, value, true);
}

void Variant::set_internal(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	set_property(_object_ref(), key, value, false);
}

Variant Variant::call(std::string_view method_name) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	Variant self = _object_ref();
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/true);
}

Variant Variant::call_internal(std::string_view method_name) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	Variant self = _object_ref();
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/false);
}

// args[0] is the object itself; it is replaced by the object adjusted to the class that declares the method.
Variant Variant::_internal_call(std::string_view method_name, std::span<Variant> args, bool enforce_public) const {
	Variant result;
	walk_hierarchy(_object_ref(), [&](const ClassInfo& info, void* object) {
		for (auto& method : info.methods) {
			if (method.name == method_name) {
				// Script-facing calls (enforce_public) only reach public methods.
				if (enforce_public && method.access != AccessLevel::Public) {
					return true;
				}
				const Callable& callable = method.callable;
				if (method.is_static) {
					result = callable.call(args.subspan(1));
				}
				else {
					args[0] = Variant(ObjectRef(object, &info));
					result = callable.call(args);
				}
				return true;
			}
		}
		return false;
	});
	return result;
}

} //namespace feather
