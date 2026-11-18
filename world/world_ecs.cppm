module;

#include <flecs.h>

export module feather.world:world_ecs;

import std;
import feather.framework;
import feather.reflection;

export namespace feather {

// Set on a component's own entity: the reflection it was registered from.
struct ReflectedComponent {
	const ClassInfo* info = nullptr;
};

// flecs' world plus registration from reflection. Copies share the underlying world (it is refcounted), so a World
// built from any flecs::world handle, such as the one flecs passes to a module, is a view of the same world.
class FEATHER_WORLD_API World : public flecs::world {
	void _link_class_info(flecs::entity_t component, const ClassInfo& info);

public:
	using flecs::world::world;

	inline World() : flecs::world() {}
	inline World(const flecs::world& other) : flecs::world(other) {}

	// Registers a value type from its ClassInfo, idempotent. The entity carries the flecs symbol of the C++ type, so
	// typed use (set<T>, queries) resolves to it in any binary, before or after this call. Null for a class that
	// cannot live in a column.
	flecs::entity register_component(const ClassInfo& info);
	// Looks the class up in ClassDB; null when unknown.
	flecs::entity register_component(StaticString class_name);
	// Every value type ClassDB knows.
	void register_reflected_components();

	// The component for a ClassDB class, found by its C++ type's symbol, or null.
	[[nodiscard]] flecs::entity get_component(StaticString class_name) const;
	// Null for components registered without reflection.
	[[nodiscard]] const ClassInfo* get_component_class_info(flecs::entity component) const;
};

// The reflected component's data on `entity`, for property access through Variant; null when the entity lacks it or
// the component has no ClassInfo. Call entity.modified(component) after writing through it.
FEATHER_WORLD_API ObjectRef get_component_ref(flecs::entity entity, flecs::entity component);
FEATHER_WORLD_API ObjectRef get_component_ref(flecs::entity entity, StaticString component);

} //namespace feather
