module feather.reflection;

import std;

namespace feather {

Variant::Variant(Reflected& ref) : Variant(&ref) {
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
	auto* obj = std::get<Reflected*>(_data);
	if (!obj) {
		return "[Object: null]";
	}
	return std::string(obj->get_class_name().str());
}

// Property/method lookup lives in ClassDB (core); without one registered, access yields NIL.
Variant Variant::get(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto* access = get_object_access();
	return access ? access->object_get(std::get<Reflected*>(_data), key, /*enforce_public=*/true) : Variant {};
}

Variant Variant::get_internal(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto* access = get_object_access();
	return access ? access->object_get(std::get<Reflected*>(_data), key, /*enforce_public=*/false) : Variant {};
}

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	if (auto* access = get_object_access()) {
		access->object_set(std::get<Reflected*>(_data), key, value, /*enforce_public=*/true);
	}
}

void Variant::set_internal(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	if (auto* access = get_object_access()) {
		access->object_set(std::get<Reflected*>(_data), key, value, /*enforce_public=*/false);
	}
}

Variant Variant::call(std::string_view method_name) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	Variant self = as<Reflected*>().value();
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/true);
}

Variant Variant::call_internal(std::string_view method_name) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	Variant self = as<Reflected*>().value();
	return _internal_call(method_name, std::span<Variant>(&self, 1), /*enforce_public=*/false);
}

Variant Variant::_internal_call(std::string_view method_name, std::span<Variant> args, bool enforce_public) const {
	auto* access = get_object_access();
	return access ? access->object_call(std::get<Reflected*>(_data), method_name, args, enforce_public) : Variant {};
}

} //namespace feather
