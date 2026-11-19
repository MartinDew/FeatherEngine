module;

#include <flecs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "entity_handle.gen.h"
#endif

export module feather.world:entity_handle;

import std;
import feather.framework;
import feather.reflection;
import :ecs_defs;

export namespace feather {

// A script-facing reference to an entity: plain data that stays valid only while its world does. Components are read
// and written by copy, so a Variant holding one never points into ECS storage.
struct FEATHER_WORLD_API EntityHandle {
	FSTRUCT(no_component);

	ecs_world_t* world = nullptr;
	uint64_t id = 0;

	EntityHandle() = default;
	EntityHandle(Entity entity) : world { entity.world().c_ptr() }, id { entity.id() } {}

	operator Entity() const { return Entity(world, id); }
	bool operator==(const EntityHandle&) const = default;

	[[method]] [[nodiscard]] bool is_valid() const;
	[[method]] void destroy();

	[[method]] [[nodiscard]] std::string get_name() const;
	[[method]] void set_name(const std::string& name);

	[[method]] [[nodiscard]] EntityHandle get_parent() const;
	// An invalid parent unparents.
	[[method]] void set_parent(const EntityHandle& parent);
	[[method]] [[nodiscard]] std::vector<EntityHandle> get_children() const;

	// `component` is a component entity, as World::get_component returns; any live entity works as a tag.
	[[method]] void add(const EntityHandle& component);
	[[method]] void remove(const EntityHandle& component);
	[[method]] [[nodiscard]] bool has(const EntityHandle& component) const;

	// A copy of the component's data as a VALUE; NIL when the entity lacks the component or it carries no data.
	[[method]] [[nodiscard]] Variant get_component(const EntityHandle& component) const;
	// Copies `value` in, which must be a VALUE of the component's class. Returns whether it was applied.
	[[method]] bool set_component(const EntityHandle& component, const Variant& value);

private:
	// The ClassInfo a component entity was registered from, or null.
	static const ClassInfo* _get_class_info(const EntityHandle& component);
};

} //namespace feather
