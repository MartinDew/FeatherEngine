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

	// Typed registration: flecs installs T's own hooks, and the component entity carries T's ClassInfo. Components
	// live at the root whichever module registers them, so they are found by class name alone.
	template <is_reflected T>
	flecs::entity register_component() {
		const flecs::entity_t scope = ecs_set_scope(c_ptr(), 0);
		const flecs::entity component = this->component<T>(T::get_class_static().data());
		ecs_set_scope(c_ptr(), scope);
		if (const ClassInfo* info = ClassDB::get_class_info<T>()) {
			_link_class_info(component.id(), *info);
		}
		return component;
	}

	// Registration from reflection alone, idempotent by name. Its hooks forward to the ClassInfo's lifecycle, and a
	// later register_component<T>() adopts the entity; a bare component<T>() does not, as it looks up its C++ name.
	// Null for classes that cannot live in a column.
	flecs::entity register_component(const ClassInfo& info);
	// Looks the class up in ClassDB; null when unknown.
	flecs::entity register_component(StaticString class_name);

	// A component registered at the root under `name`, or null.
	[[nodiscard]] flecs::entity get_component(StaticString name) const;
	// Null for components registered without reflection.
	[[nodiscard]] const ClassInfo* get_component_class_info(flecs::entity component) const;
};

// Reads and writes one property of a reflected component on `entity`, by name. Only public properties are reachable;
// a get on a missing component or property returns NIL.
FEATHER_WORLD_API Variant get_component_property(flecs::entity entity, flecs::entity component, StaticString property);
FEATHER_WORLD_API bool set_component_property(
		flecs::entity entity,
		flecs::entity component,
		StaticString property,
		const Variant& value
);

FEATHER_WORLD_API Variant get_component_property(flecs::entity entity, StaticString component, StaticString property);
FEATHER_WORLD_API bool set_component_property(
		flecs::entity entity,
		StaticString component,
		StaticString property,
		const Variant& value
);

} //namespace feather
