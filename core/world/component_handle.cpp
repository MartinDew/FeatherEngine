#include "component_handle.h"

#include <main/class_db.h>

namespace feather {

namespace {

// A typed read, falling back to `fallback` when the property is missing or
// holds another type -- a name from a script is a normal mistake, not a fault.
template <typename T>
T read(const ComponentHandle& handle, std::string_view property, T fallback) {
	const Variant value = handle.get(property);
	if (auto converted = value.as<T>()) {
		return converted.value();
	}
	return fallback;
}

} //namespace

bool ComponentHandle::is_valid() const {
	return _world != nullptr && _world->has_component(_entity, _component);
}

std::string ComponentHandle::get_type_name() const {
	return std::string(_component.str());
}

const void* ComponentHandle::data() const {
	return _world ? _world->component_data(_entity, _component) : nullptr;
}

void* ComponentHandle::mutable_data() {
	return _world ? _world->mutable_component_data(_entity, _component) : nullptr;
}

Variant ComponentHandle::get(std::string_view property) const {
	if (!_world) {
		return {};
	}
	return _world->get_property(_entity, _component, property);
}

bool ComponentHandle::set(std::string_view property, const Variant& value) {
	return _world != nullptr && _world->set_property(_entity, _component, property, value);
}

bool ComponentHandle::get_bool(std::string property) const {
	return read<bool>(*this, property, false);
}

int ComponentHandle::get_int(std::string property) const {
	return read<int>(*this, property, 0);
}

real_t ComponentHandle::get_float(std::string property) const {
	return read<real_t>(*this, property, 0);
}

Vector2 ComponentHandle::get_vector2(std::string property) const {
	return read<Vector2>(*this, property, Vector2 {});
}

Vector3 ComponentHandle::get_vector3(std::string property) const {
	return read<Vector3>(*this, property, Vector3 {});
}

Color ComponentHandle::get_color(std::string property) const {
	return read<Color>(*this, property, Color {});
}

bool ComponentHandle::set_bool(std::string property, bool value) {
	return set(property, Variant(value));
}

bool ComponentHandle::set_int(std::string property, int value) {
	return set(property, Variant(value));
}

bool ComponentHandle::set_float(std::string property, real_t value) {
	return set(property, Variant(value));
}

bool ComponentHandle::set_vector2(std::string property, Vector2 value) {
	return set(property, Variant(value));
}

bool ComponentHandle::set_vector3(std::string property, Vector3 value) {
	return set(property, Variant(value));
}

bool ComponentHandle::set_color(std::string property, Color value) {
	return set(property, Variant(value));
}

} //namespace feather
