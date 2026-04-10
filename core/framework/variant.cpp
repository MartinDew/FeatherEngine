#include "variant.h"

#include "assert.h"
#include "framework/reflected.h"
#include <main/class_db.h>

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
			_data, other._data);
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
		return std::to_string(std::get<real_t>(_data));
	case VariantType::STRING:
		return std::get<std::string>(_data);
	case VariantType::ARRAY:
		return "[Array]";
	case VariantType::OBJECT:
		return "[Object] : " + get_name();
	case VariantType::INVALID:
		return "[Invalid]";
	default:;
	}
	return "[Unknown]";
}

std::string Variant::get_name() const {
	if (_type != VariantType::OBJECT) {
		return to_string();
	}

	return _object_info->name;
}

Variant Variant::get(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto info = _object_info;
	for (auto& property : info->properties) {
		if (property.name == key) {
			return property.getter(std::get<Reflected*>(_data));
		}
	}
}

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto info = _object_info;
	for (auto& property : info->properties) {
		if (property.name == key) {
			property.setter(std::get<Reflected*>(_data), value);
			return;
		}
	}
}

void Variant::set_class_info(StaticString class_name) {
	_object_info = ClassDB::get()._get_class_info_internal(class_name);
}

Variant Variant::_internal_call(std::string_view method_name, std::span<Variant> args) const {
	auto info = _object_info;
	while (info) {
		for (auto& method : info->methods) {
			if (method.name == method_name) {
				Callable& callable = method.callable;

				return callable.call(args);
			}
		}
		info = ClassDB::_get_class_info_internal(info->parent);
	}

	return {};
}

Variant Variant::call(std::string_view method_name) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto info = _object_info;
	while (info) {
		for (auto& method : info->methods) {
			if (method.name == method_name) {
				Callable& callable = method.callable;
				return callable.call(as<Reflected*>().value());
			}
		}
		info = ClassDB::_get_class_info_internal(info->parent);
	}
	return {};
}

} //namespace feather