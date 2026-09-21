#pragma once

#include "ecs_defs.h"

#include <framework/class_info.h>
#include <framework/delegate.h>
#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <framework/static_string.hpp>
#include <framework/variant.h>

#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <unordered_map>

#ifndef FEATHER_REFLECTION_PARSER
#include "world.gen.h"
#endif

namespace feather {

class Entity;
class Component;
class EcsModule;

template <class... TComps>
class Query;

template <class... TComps>
class SystemBuilder;

// A type ClassDB knows by name. ClassDB is the source of truth for that name: a type it describes is registered with
// the ECS under its class name and never under a second spelling.
template <typename T>
concept ClassDbNamed = requires {
	{ T::get_class_static() } -> std::convertible_to<StaticString>;
};

// An ECS world: flecs underneath, Feather's own vocabulary on top, and no flecs type in this header.
// It knows nothing of ClassDB until register_classdb_components or import_classdb_modules is called.
class FEATHER_API World : public Reflected {
	// abstract: reflected so its API is reachable by name, never built through ClassDB -- a world has one owner.
	FCLASS(abstract);

	// Companion classes, split out for file organization only; these grant the access that split would otherwise cost.
	friend class Entity;
	template <class... TComps>
	friend class Query;
	template <class... TComps>
	friend class SystemBuilder;

	struct Impl;
	std::unique_ptr<Impl> _impl;

	// Component id per registered class name, and the record of what is already registered: flecs would otherwise
	// define the same name twice.
	std::unordered_map<StaticString, EntityId> _components;

	// Modules already imported, by class name. The sweep and the delegate can both reach the same module.
	std::unordered_map<StaticString, EntityId> _modules;

	Delegate<std::string_view>::id_t _component_delegate = -1;
	Delegate<std::string_view>::id_t _module_delegate = -1;

	// flecs keeps the `const char*` it is handed and StaticString is a non-owning view, so a name that came from a
	// std::string needs somewhere permanent to point.
	static StaticString _intern(std::string_view name);

	// Opens a module scope named after `class_name`, returning the scope to restore.
	EntityId _begin_module(StaticString class_name, EntityId& out_module);
	void _end_module(EntityId previous_scope);

	// Registers a component from storage alone, for a type ClassDB cannot describe.
	EntityId _register_component_raw(StaticString name, const ValueTypeOps& ops);

	// Copies `size` bytes into the component's storage, adding it if absent. What Entity::set<T> reduces to.
	bool _set_component_raw(EntityId entity, StaticString class_name, const void* value, size_t size);

	// Non-template on purpose: the builders describe what they want as plain data and hand it over here, which is what
	// keeps flecs out of their headers.
	EntityId _register_system(SystemDesc&& desc);

	// Returns an opaque ecs_query_t*, owned by the caller -- Query's destructor hands it back to _destroy_query.
	[[nodiscard]] void* _create_query(QueryDesc&& desc);
	void _destroy_query(void* query) const;

	// Runs `callback` once per matching batch. Synchronous, so `ctx` need only outlive the call.
	void _query_each(void* query, SystemCallback callback, void* ctx);

public:
	World();
	~World() override;

	// Neither copyable nor movable: a world that has been asked to watch ClassDB holds delegates capturing `this`.
	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	// ---- Simulation --------------------------------------------------------

	// Runs the active pipeline once. A `delta` of 0 lets flecs measure the frame itself.
	[[method]] bool progress(double delta = 0.0);

	// A timer entity ticking every `interval` seconds, for a system that runs on a fixed step rather than every frame.
	[[nodiscard]] EntityId create_timer(double interval);

	// Time elapsed in the frame being simulated, so an each() callback need not take an iterator to know the timestep.
	[[nodiscard]] float delta_time() const;

	// The flecs REST explorer, for debug builds.
	void enable_rest_api();

	// ---- Pipelines ---------------------------------------------------------

	// The entity for one of the phases the default pipeline runs. What SystemBuilder::phase resolves a SystemPhase to.
	[[nodiscard]] EntityId phase(SystemPhase phase) const;

	// A phase of your own, running after `runs_after`. Order comes from that dependency chain rather than from the
	// pipeline, which only decides which systems it considers at all.
	[[nodiscard]] EntityId create_phase(EntityId runs_after);

	// A pipeline matching every system that has a phase, walked in phase order -- the shape the default one has.
	// `only_tagged` narrows it to systems carrying that tag, which is how a second pipeline gets its own set.
	[[nodiscard]] EntityId create_pipeline(const std::string& name, EntityId only_tagged = {});

	// The pipeline progress() runs. Starts as the default one, whose phases SystemPhase mirrors.
	[[nodiscard]] EntityId get_pipeline() const;
	void set_pipeline(EntityId pipeline);

	// ---- Entities ----------------------------------------------------------

	[[nodiscard]] Entity create_entity(const std::string& name = "");
	[[nodiscard]] Entity create_entity(const Entity& parent, const std::string& name = "");
	[[nodiscard]] Entity entity(EntityId id);
	[[nodiscard]] Entity prefab(const std::string& name);
	[[nodiscard]] Entity lookup(const std::string& name);
	void destroy_entity(EntityId id) const;
	[[nodiscard]] bool is_valid(EntityId id) const;

	// An entity's own attributes. Entity is a handle with no storage, so everything it reports goes through here rather
	// than through a flecs handle it would have to rebuild on every call.
	[[nodiscard]] std::string get_entity_name(EntityId id) const;
	void set_entity_name(EntityId id, const std::string& name) const;
	[[nodiscard]] EntityId get_parent(EntityId id) const;
	void set_parent(EntityId child, EntityId parent) const;
	// Makes `entity` an instance of `prefab`, inheriting whatever the prefab declares.
	void set_prefab(EntityId entity, EntityId prefab) const;
	[[nodiscard]] bool is_instance_of(EntityId entity, EntityId prefab) const;

	// ---- Component types ---------------------------------------------------

	// Registers every IComponent subclass ClassDB knows, then keeps listening so one arriving later -- from a project
	// DLL, say -- registers too. Opt-in: a world is tied to the reflected type registry only once asked.
	void register_classdb_components();

	// Registers `class_name` with the ECS under exactly that name, using the storage its ClassInfo describes.
	// Idempotent, and returns a null id when the class is unknown or is not a value type.
	EntityId register_component(StaticString class_name);

	// The id a class name was registered under, or a null id.
	[[nodiscard]] EntityId find_component(StaticString class_name) const;

	// One field of a component being described at runtime.
	struct FieldDesc {
		std::string name;
		VariantType type = VariantType::NIL;
	};

	// Registers a component whose fields are described at runtime rather than by a C++ type, laying its storage out
	// here. Returns a null id with *error set on a bad description, since that comes from outside the engine.
	EntityId register_component(const std::string& name, std::span<const FieldDesc> fields, std::string* error = nullptr);

	// Registers a reflected type by its ClassDB name. No name parameter on purpose: ClassDB owns the spelling.
	template <ClassDbNamed T>
	EntityId register_component_type();

	// Registers a C++ type ClassDB cannot describe, and so can never arrive through a subclass delegate -- the
	// third-party math types are why this exists (see ecs/math_module.cpp).
	template <typename T>
		requires(!ClassDbNamed<T>)
	EntityId register_component_type(const char* name);

	// ---- Components on entities, by name -----------------------------------

	bool add_component(EntityId entity, StaticString class_name);
	[[nodiscard]] bool has_component(EntityId entity, StaticString class_name) const;
	void remove_component(EntityId entity, StaticString class_name);
	void remove_all(StaticString class_name);

	// Raw storage for a component instance, or nullptr. The mutable one also marks it changed.
	[[nodiscard]] const void* component_data(EntityId entity, StaticString class_name) const;
	[[nodiscard]] void* mutable_component_data(EntityId entity, StaticString class_name);

	// Named property access lives on Component (ecs/component.h), which caches the ClassInfo and the component id
	// rather than re-resolving both on every field touch.

	// ---- Systems -----------------------------------------------------------

	// Declares a system against this world. Anything holding a World can declare one; EcsModule is a convenience for
	// grouping them, not a gate. Defined in ecs/system_builder.h, which a caller needs for the builder anyway.
	template <class... TComps>
	[[nodiscard]] SystemBuilder<TComps...> system(const char* name);

	// ---- Modules -----------------------------------------------------------

	// Imports every EcsModule subclass ClassDB knows, then keeps listening for ones arriving later. Opt-in for the same
	// reason as register_classdb_components.
	void import_classdb_modules();

	// Builds the module ClassDB registered under `class_name` and lets it declare what it owns, once, under a module
	// scope of that name. Returns a null id when the name is not a module ClassDB can build.
	EntityId import_module(StaticString class_name);

	// The same for a module named by C++ type: its ClassDB name is what identifies it either way.
	template <ClassDbNamed T>
	EntityId import_module() {
		return import_module(T::get_class_static());
	}

	[[nodiscard]] bool is_module_imported(StaticString class_name) const;

	// ---- Reflected surface -------------------------------------------------
	// Names, never ids: an EntityId is not Variant-marshalable, so a script reaches entities through Entity instead.

	[[method]] bool has_component_type(std::string class_name) const;
	[[method]] bool register_component_type_by_name(std::string class_name);
	[[method]] bool has_module(std::string class_name) const;
	[[method]] int get_component_type_count() const;
};

template <ClassDbNamed T>
EntityId World::register_component_type() {
	return register_component(T::get_class_static());
}

template <typename T>
	requires(!ClassDbNamed<T>)
EntityId World::register_component_type(const char* name) {
	const StaticString interned = _intern(name);
	if (const EntityId existing = find_component(interned); !existing.is_null()) {
		return existing;
	}
	return _register_component_raw(interned, make_value_type_ops<T>());
}

} //namespace feather
