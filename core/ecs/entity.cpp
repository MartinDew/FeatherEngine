#include "entity.h"

#include "component.h"

namespace feather {

bool Entity::is_valid() const {
	return _world != nullptr && _world->is_valid(_id);
}

std::string Entity::get_name() const {
	if (!is_valid()) {
		return {};
	}
	return _world->get_entity_name(_id);
}

void Entity::set_name(std::string name) {
	if (is_valid()) {
		_world->set_entity_name(_id, name);
	}
}

void Entity::destroy() {
	if (is_valid()) {
		_world->destroy_entity(_id);
	}
	_id = {};
}

Entity& Entity::child_of(const Entity& parent) {
	if (is_valid()) {
		_world->set_parent(_id, parent._id);
	}
	return *this;
}

Entity& Entity::is_a(const Entity& prefab) {
	if (is_valid()) {
		_world->set_prefab(_id, prefab._id);
	}
	return *this;
}

Entity Entity::get_parent() const {
	if (!is_valid()) {
		return {};
	}
	return { *_world, _world->get_parent(_id) };
}

bool Entity::is_descendant_of(const Entity& ancestor) const {
	if (!is_valid() || !ancestor.is_valid()) {
		return false;
	}
	// Walks up rather than querying: a scene's depth is small, and this is also what decides whether an entity
	// belongs to the active scene.
	for (EntityId current = _id; !current.is_null(); current = _world->get_parent(current)) {
		if (current == ancestor._id) {
			return true;
		}
	}
	return false;
}

bool Entity::add_component(std::string component) {
	return is_valid() && _world->add_component(_id, World::_intern(component));
}

bool Entity::has_component(std::string component) const {
	return is_valid() && _world->has_component(_id, StaticString(component));
}

void Entity::remove_component(std::string component) {
	if (is_valid()) {
		_world->remove_component(_id, StaticString(component));
	}
}

Component Entity::component(std::string_view component) const {
	if (!is_valid()) {
		return {};
	}
	return { *_world, _id, StaticString(component) };
}

} //namespace feather
