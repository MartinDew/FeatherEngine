#pragma once

#include "ecs_defs.h"
#include "world.h"

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#include <string>
#include <utility>

#ifndef FEATHER_REFLECTION_PARSER
#include "entity.gen.h"
#endif

namespace feather {

class ComponentHandle;

// A handle to one entity in a World: the front door to the ECS.
//
// It owns nothing and costs a pointer and an id, so it is passed by value. The
// reflected half names components and properties by string, which is what a
// script or an editor has; the templated half takes C++ types and is what
// engine code uses. Both end up in the same place.
//
// Distinct from EcsEntity (ecs_defs.h), which is flecs' own handle -- a system
// callback is handed one of those by flecs itself.
class Entity : public Reflected {
	FCLASS();

	World* _world = nullptr;
	Ecs::entity_t _id = 0;

public:
	Entity() = default;
	Entity(World& world, Ecs::entity_t id) : _world(&world), _id(id) {}
	Entity(World& world, const EcsEntity& entity) : _world(&world), _id(entity.id()) {}

	// ---- Identity ----------------------------------------------------------

	[[nodiscard]] Ecs::entity_t id() const { return _id; }
	[[nodiscard]] World* get_world() const { return _world; }
	// flecs' own handle, for the query and system builders World does not wrap.
	[[nodiscard]] EcsEntity ecs() const;

	explicit operator bool() const { return is_valid(); }
	bool operator==(const Entity& o) const { return _world == o._world && _id == o._id; }
	bool operator!=(const Entity& o) const { return !(*this == o); }

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

	// A handle to one of this entity's components, valid only while the
	// component is present.
	[[nodiscard]] ComponentHandle component(std::string_view component) const;

	// ---- Components, by type -----------------------------------------------

	template <typename T>
	Entity& add();

	template <typename T>
	Entity& set(const T& value);

	// Constructs the component in place, like flecs' own emplace.
	template <typename T, typename... TArgs>
	Entity& emplace(TArgs&&... args);

	template <typename T>
	[[nodiscard]] const T* get() const;

	template <typename T>
	[[nodiscard]] bool has() const;

	template <typename T>
	Entity& remove();
};

template <typename T>
Entity& Entity::add() {
	ecs().template add<T>();
	return *this;
}

template <typename T>
Entity& Entity::set(const T& value) {
	ecs().template set<T>(value);
	return *this;
}

template <typename T, typename... TArgs>
Entity& Entity::emplace(TArgs&&... args) {
	ecs().template emplace<T>(std::forward<TArgs>(args)...);
	return *this;
}

template <typename T>
const T* Entity::get() const {
	return ecs().template get<T>();
}

template <typename T>
bool Entity::has() const {
	return ecs().template has<T>();
}

template <typename T>
Entity& Entity::remove() {
	ecs().template remove<T>();
	return *this;
}

} //namespace feather
