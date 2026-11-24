module;

#include <flecs.h>

module feather.world;

import std;

namespace feather {

const ClassInfo* EntityHandle::_get_class_info(const EntityHandle& component) {
	if (!component.is_valid()) {
		return nullptr;
	}
	const ReflectedComponent* reflected = Entity(component).try_get<ReflectedComponent>();
	return reflected ? reflected->get_class_info() : nullptr;
}

bool EntityHandle::is_valid() const {
	return world && id != 0 && ecs_is_alive(world, id);
}

void EntityHandle::destroy() {
	if (is_valid()) {
		Entity(*this).destruct();
	}
}

std::string EntityHandle::get_name() const {
	const char* name = is_valid() ? ecs_get_name(world, id) : nullptr;
	return name ? std::string(name) : std::string {};
}

void EntityHandle::set_name(const std::string& name) {
	if (is_valid()) {
		Entity(*this).set_name(name.c_str());
	}
}

EntityHandle EntityHandle::get_parent() const {
	return is_valid() ? EntityHandle(Entity(*this).parent()) : EntityHandle {};
}

void EntityHandle::set_parent(const EntityHandle& parent) {
	if (!is_valid()) {
		return;
	}
	if (parent.is_valid()) {
		Entity(*this).child_of(Entity(parent));
	}
	else {
		Entity(*this).remove(flecs::ChildOf, flecs::Wildcard);
	}
}

std::vector<EntityHandle> EntityHandle::get_children() const {
	std::vector<EntityHandle> children;
	if (is_valid()) {
		Entity(*this).children([&children](Entity child) { children.emplace_back(child); });
	}
	return children;
}

void EntityHandle::add(const EntityHandle& component) {
	if (is_valid() && component.is_valid()) {
		Entity(*this).add(Entity(component));
	}
}

void EntityHandle::remove(const EntityHandle& component) {
	if (is_valid() && component.is_valid()) {
		Entity(*this).remove(Entity(component));
	}
}

bool EntityHandle::has(const EntityHandle& component) const {
	return is_valid() && component.is_valid() && Entity(*this).has(Entity(component));
}

Variant EntityHandle::get_component(const EntityHandle& component) const {
	const ClassInfo* info = is_valid() ? _get_class_info(component) : nullptr;
	// A tag has no storage to copy.
	if (!info || info->is_empty) {
		return {};
	}
	const void* data = ecs_get_id(world, id, component.id);
	return data ? Variant::from_value(*info, data) : Variant {};
}

bool EntityHandle::set_component(const EntityHandle& component, const Variant& value) {
	const ClassInfo* info = is_valid() ? _get_class_info(component) : nullptr;
	const Variant::Instance instance = value.get_instance();
	// A built-in's ClassInfo is built per call, so classes are matched by type rather than by address.
	if (!info || !instance.info || instance.info->cpp_name != info->cpp_name) {
		return false;
	}
	if (info->is_empty) {
		Entity(*this).add(Entity(component));
		return true;
	}
	// flecs copies through the component's hooks and notifies observers.
	ecs_set_id(world, id, component.id, info->size, instance.data);
	return true;
}

} //namespace feather
