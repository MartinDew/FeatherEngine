#pragma once

#include "ecs_defs.h"
#include "world.h"

#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#include <string>
#include <utility>

#ifndef FEATHER_REFLECTION_PARSER
#include "entity.gen.h"
#endif

namespace feather {

class Component;

// A handle to one entity in a World: a pointer and an id, passed by value. The reflected half names components by
// string and the templated half by C++ type; both reduce to World's by-name layer, so neither needs flecs.
class FEATHER_API Entity : public Reflected {
	FCLASS();

	World* _world = nullptr;
	EntityId _id;

public:
	Entity() = default;
	Entity(World& world, EntityId id) : _world(&world), _id(id) {}

	// ---- Identity ----------------------------------------------------------

	[[nodiscard]] EntityId id() const { return _id; }
	[[nodiscard]] World* get_world() const { return _world; }

	explicit operator bool() const { return is_valid(); }
	bool operator==(const Entity& other) const { return _world == other._world && _id == other._id; }
	bool operator!=(const Entity& other) const { return !(*this == other); }

	[[method]] bool is_valid() const;
	[[method]] std::string get_name() const;
	[[method]] void set_name(std::string name);
	[[method]] void destroy();

	// ---- Hierarchy ---------------------------------------------------------

	Entity& child_of(const Entity& parent);
	Entity& is_a(const Entity& prefab);
	[[nodiscard]] Entity get_parent() const;
	// True when `ancestor` is this entity or one of its parents.
	[[method]] bool is_descendant_of(const Entity& ancestor) const;

	// ---- Components, by name -----------------------------------------------

	[[method]] bool add_component(std::string component);
	[[method]] bool has_component(std::string component) const;
	[[method]] void remove_component(std::string component);

	// A handle to one of this entity's components, valid only while the component is present.
	[[nodiscard]] Component component(std::string_view component) const;

	// ---- Components, by type -----------------------------------------------
	// Each reduces to the by-name layer using the type's reflected class name, so none of them needs flecs.

	template <typename T>
	Entity& add();

	template <typename T>
	Entity& set(const T& value);

	// Constructs the component in place and moves it in. Named for flecs' own emplace, though unlike flecs' it does
	// build a temporary: constructing directly into table storage would mean naming the table.
	template <typename T, typename... TArgs>
	Entity& emplace(TArgs&&... args);

	template <typename T>
	[[nodiscard]] const T* get() const;

	template <typename T>
	[[nodiscard]] T* get_mutable();

	template <typename T>
	[[nodiscard]] bool has() const;

	template <typename T>
	Entity& remove();
};

template <typename T>
Entity& Entity::add() {
	if (_world) {
		_world->add_component(_id, T::get_class_static());
	}
	return *this;
}

template <typename T>
Entity& Entity::set(const T& value) {
	if (_world) {
		_world->_set_component_raw(_id, T::get_class_static(), &value, sizeof(T));
	}
	return *this;
}

template <typename T, typename... TArgs>
Entity& Entity::emplace(TArgs&&... args) {
	T value(std::forward<TArgs>(args)...);
	return set<T>(value);
}

template <typename T>
const T* Entity::get() const {
	return _world ? static_cast<const T*>(_world->component_data(_id, T::get_class_static())) : nullptr;
}

template <typename T>
T* Entity::get_mutable() {
	return _world ? static_cast<T*>(_world->mutable_component_data(_id, T::get_class_static())) : nullptr;
}

template <typename T>
bool Entity::has() const {
	return _world != nullptr && _world->has_component(_id, T::get_class_static());
}

template <typename T>
Entity& Entity::remove() {
	if (_world) {
		_world->remove_component(_id, T::get_class_static());
	}
	return *this;
}

} //namespace feather
