module;

#include <flecs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "world_ecs.gen.h"
#endif

export module feather.world:world_ecs;

import std;
import feather.framework;
import feather.reflection;
import :ecs_defs;

export namespace feather {

// Set on a component's own entity: the reflection it was registered from.
struct ReflectedComponent {
	const ClassInfo* info = nullptr;
};

// flecs' world plus reflection-driven registration. A copy, or a World built from any flecs::world handle, views the
// same refcounted world. Reflected is the first base because codegen takes the first base as the reflected parent.
class FEATHER_WORLD_API World : public Reflected, public flecs::world {
	FCLASS();

	void _link_class_info(flecs::entity_t component, const ClassInfo& info);

public:
	using flecs::world::world;

	inline World() : flecs::world() {}
	inline World(const flecs::world& other) : flecs::world(other) {}

	// Registers a value type from its ClassInfo, idempotent. The entity carries the flecs symbol of the C++ type, so
	// typed use (set<T>, queries) resolves to it in any binary, before or after this call. Null for a class that
	// cannot live in a column.
	Entity register_component(const ClassInfo& info);
	// Looks the class up in ClassDB; null when unknown.
	Entity register_component(StaticString class_name);
	// Every value type ClassDB knows.
	void register_reflected_components();

	// The component for a ClassDB class, found by its C++ type's symbol, or null.
	[[nodiscard]] Entity get_component(StaticString class_name) const;
	// Null for components registered without reflection.
	[[nodiscard]] const ClassInfo* get_component_class_info(Entity component) const;
};

} //namespace feather
