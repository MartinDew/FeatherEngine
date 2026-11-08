module feather.core;

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

std::string Variant::to_string() const {
	switch (_type) {
	case VariantType::NIL:
		return "nil";
	case VariantType::BOOL:
		return std::get<bool>(_data) ? "true" : "false";
	case VariantType::INT:
		return std::to_string(std::get<int>(_data));
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

// Walks from the object's class up to its root, handing each level's ClassInfo and the object pointer adjusted to
// that level to visit; stops at the first level visit returns true for.
template <class TVisit>
bool walk_hierarchy(ObjectRef ref, TVisit&& visit) {
	const ClassInfo* info = ref.get_class_info();
	void* object = ref.get();
	while (info && object) {
		if (visit(*info, object)) {
			return true;
		}
		if (info->to_parent) {
			object = info->to_parent(object);
		}
		info = ClassDB::get_class_info(info->parent);
	}
	return false;
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

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	set_property(_object_ref(), key, value, true);
}

void Variant::set_internal(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	set_property(_object_ref(), key, value, false);
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
