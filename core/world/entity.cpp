#include "entity.h"

#include "component_handle.h"

namespace feather {

EcsEntity Entity::ecs() const {
	return _world ? _world->ecs().entity(_id) : EcsEntity();
}

bool Entity::is_valid() const {
	return _world != nullptr && _world->is_valid(_id);
}

std::string Entity::get_name() const {
	if (!is_valid()) {
		return {};
	}
	const char* name = ecs().name();
	return name ? name : std::string {};
}

void Entity::set_name(std::string name) {
	if (is_valid()) {
		ecs().set_name(name.c_str());
	}
}

void Entity::destroy() {
	if (is_valid()) {
		_world->destroy_entity(_id);
	}
	_id = 0;
}

Entity& Entity::child_of(const Entity& parent) {
	if (is_valid()) {
		ecs().child_of(parent.ecs());
	}
	return *this;
}

Entity& Entity::is_a(const Entity& prefab) {
	if (is_valid()) {
		ecs().is_a(prefab.ecs());
	}
	return *this;
}

Entity Entity::get_parent() const {
	if (!is_valid()) {
		return {};
	}
	return { *_world, ecs().parent() };
}

bool Entity::is_descendant_of(const Entity& ancestor) const {
	if (!is_valid() || !ancestor.is_valid()) {
		return false;
	}
	// Walks up rather than querying: a scene's depth is small, and this is also
	// what decides whether an entity belongs to the active scene.
	for (EcsEntity current = ecs(); current.is_valid(); current = current.parent()) {
		if (current.id() == ancestor.id()) {
			return true;
		}
	}
	return false;
}

bool Entity::add_component(std::string component) {
	return is_valid() && _world->add_component(_id, StaticString(component));
}

bool Entity::has_component(std::string component) const {
	return is_valid() && _world->has_component(_id, StaticString(component));
}

void Entity::remove_component(std::string component) {
	if (is_valid()) {
		_world->remove_component(_id, StaticString(component));
	}
}

ComponentHandle Entity::component(std::string_view component) const {
	if (!is_valid()) {
		return {};
	}
	return { *_world, _id, StaticString(component) };
}

} //namespace feather
