#include "component.h"

#include <main/class_db.h>

namespace feather {

Component::Component(World& world, EntityId entity, StaticString type_name) :
	_world(&world), _entity(entity), _type_name(type_name) {
	// The introspection the container exists for, resolved once here rather than on every field access. Both can come
	// back empty for a name nothing has registered, which is what is_valid() reports.
	_info = ClassDB::get_class_info(type_name);
	_component_id = world.find_component(type_name);
}

const ClassInfo::Property* Component::_find_property(std::string_view name) const {
	if (!_info) {
		return nullptr;
	}
	for (const ClassInfo::Property& property : _info->properties) {
		if (property.name == name) {
			return &property;
		}
	}
	return nullptr;
}

bool Component::is_valid() const {
	return _world != nullptr && !_component_id.is_null() && _world->has_component(_entity, _type_name);
}

std::string Component::get_type_name() const {
	return std::string(_type_name.str());
}

const void* Component::data() const {
	// Deliberately not cached: adding or removing any component moves the entity to a different archetype and
	// relocates this storage, so a pointer held across such a change would dangle.
	return _world ? _world->component_data(_entity, _type_name) : nullptr;
}

void* Component::mutable_data() {
	return _world ? _world->mutable_component_data(_entity, _type_name) : nullptr;
}

Variant Component::get(std::string_view property) const {
	const ClassInfo::Property* found = _find_property(property);
	if (!found || !found->getter) {
		return {};
	}
	const void* storage = data();
	if (!storage) {
		return {};
	}
	// A value type's accessors take the instance itself, not a Reflected*.
	return found->getter(const_cast<void*>(storage));
}

bool Component::set(std::string_view property, const Variant& value) {
	const ClassInfo::Property* found = _find_property(property);
	if (!found || !found->setter) {
		return false;
	}
	void* storage = mutable_data();
	if (!storage) {
		return false;
	}
	found->setter(storage, value);
	return true;
}

} //namespace feather
