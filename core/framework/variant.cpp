#include "variant.h"

#include "assert.h"
#include <main/class_db.h>

namespace feather {

Variant::Variant(Reflected& ref) : Variant(&ref) {}

std::string Variant::to_string() const {
	switch (_type) {
	case VariantType::NIL:
		return "nil";
	case VariantType::BOOL:
		return std::get<bool>(_data) ? "true" : "false";
	case VariantType::INT:
		return std::to_string(std::get<size_t>(_data));
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
	}
	return "[Unknown]";
}

std::string Variant::get_name() const {
	if (_type != VariantType::OBJECT) {
		return to_string();
	}

	return static_cast<ClassDB::ClassInfo*>(_object_info)->name;
}

Variant Variant::get(std::string_view key) const {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto info = static_cast<ClassDB::ClassInfo*>(_object_info);
	for (auto& property : info->properties) {
		if (property.name == key) {
			return property.getter(std::get<Reflected*>(_data));
		}
	}
}

void Variant::set(std::string_view key, const Variant& value) {
	fassert(_type == VariantType::OBJECT, "Variant is not an object");
	auto info = static_cast<ClassDB::ClassInfo*>(_object_info);
	for (auto& property : info->properties) {
		if (property.name == key) {
			property.setter(std::get<Reflected*>(_data), value);
			return;
		}
	}
}

void Variant::set_class_info(StaticString class_name) {
	_object_info = &ClassDB::get()._get_class_info_internal(class_name);
}

} //namespace feather