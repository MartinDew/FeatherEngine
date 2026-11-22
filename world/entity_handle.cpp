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
	return reflected ? reflected->info : nullptr;
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
	return is_valid() ? std::string(Entity(*this).name().c_str()) : std::string {};
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

namespace {

const BuiltinComponent* get_builtin(const EntityHandle& component) {
	return component.is_valid() ? Entity(component).try_get<BuiltinComponent>() : nullptr;
}

} // namespace

Variant EntityHandle::get_component(const EntityHandle& component) const {
	if (!is_valid()) {
		return {};
	}
	if (const BuiltinComponent* builtin = get_builtin(component)) {
		return Variant::from_builtin(builtin->type, ecs_get_id(world, id, component.id));
	}
	const ClassInfo* info = _get_class_info(component);
	// A tag has no storage to copy.
	if (!info || info->is_empty) {
		return {};
	}
	const void* data = ecs_get_id(world, id, component.id);
	return data ? Variant::from_value(*info, data) : Variant {};
}

Variant EntityHandle::get_component_ref(const EntityHandle& component) {
	const ClassInfo* info = is_valid() ? _get_class_info(component) : nullptr;
	if (!info || info->is_empty) {
		return {};
	}
	void* data = ecs_get_mut_id(world, id, component.id);
	return data ? Variant::ref(*info, data) : Variant {};
}

bool EntityHandle::set_component(const EntityHandle& component, const Variant& value) {
	if (!is_valid()) {
		return false;
	}
	if (const BuiltinComponent* builtin = get_builtin(component)) {
		const ecs_type_info_t* type_info = ecs_get_type_info(world, component.id);
		if (value.get_type() != builtin->type || !type_info) {
			return false;
		}
		ecs_set_id(world, id, component.id, static_cast<size_t>(type_info->size), value.builtin_data());
		return true;
	}
	const ClassInfo* info = _get_class_info(component);
	const ValueBox* box = value.get_value();
	if (!info || !box || box->get_class_info() != info) {
		return false;
	}
	if (info->is_empty) {
		Entity(*this).add(Entity(component));
		return true;
	}
	// flecs copies through the component's hooks and notifies observers.
	ecs_set_id(world, id, component.id, info->size, box->get());
	return true;
}

} //namespace feather
