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
import :entity_handle;

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
	// feather.<module_name>, created on first use.
	flecs::entity_t _module_scope(StaticString module_name);
	// Moves an entity typed flecs use created to feather.<module_name>.<name>, wherever it was made.
	void _place(flecs::entity_t entity, StaticString module_name, StaticString name);

public:
	using flecs::world::world;

	inline World() : flecs::world() {}
	inline World(const flecs::world& other) : flecs::world(other) {}

	// Registers a value type at feather.<module>.<class>, idempotent. Typed use (set<T>, queries) in any binary
	// resolves to it by the C++ type's flecs symbol, before or after this call. Null for a if the type cannot live in a column.
	Entity register_component(const ClassInfo& info);
	// Looks the class up in ClassDB; null when unknown.
	Entity register_component(StaticString class_name);
	// Every value type ClassDB knows, after the components that describe them.
	void register_reflected_components();

	// A type Variant stores natively, at feather.<module>.<name> from its FType, readable through EntityHandle.
	template <class T>
	Entity register_builtin_component() {
		const Entity c = component<T>();
		_place(c, FType<T>::module_name, FType<T>::name);
		_link_class_info(c, *Variant::get_builtin_class_info(get_variant_type<T>()));
		return c;
	}

	// Hides flecs' import: a reflected module's entity, with the systems it made, moves to feather.<module>.<class>.
	template <class T>
	Entity import() {
		const Entity m = flecs::world::import<T>();
		if constexpr (requires { FType<T>::name; }) {
			_place(m, FType<T>::module_name, FType<T>::name);
		}
		return m;
	}

	[[method]] [[nodiscard]] EntityHandle create_entity(const std::string& name);
	[[method]] [[nodiscard]] EntityHandle get_entity(const std::string& path) const;
	// The component for a ClassDB class, found by its C++ type's symbol, or for a builtin type by name; invalid when
	// there is none.
	[[method]] [[nodiscard]] EntityHandle get_component(const std::string& class_name) const;
	// Null for components registered without reflection.
	[[nodiscard]] const ClassInfo* get_component_class_info(Entity component) const;
};

} //namespace feather
