#pragma once

#include "ecs_defs.h"

#include <framework/class_info.h>
#include <framework/delegate.h>
#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>
#include <framework/static_string.hpp>
#include <framework/variant.h>

#include <memory>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

#ifndef FEATHER_REFLECTION_PARSER
#include "world.gen.h"
#endif

namespace feather {

class Entity;
class Component;

// The engine's ECS world: flecs underneath, Feather's own vocabulary on top.
//
// What it adds over flecs is registration by reflection. A component type is an IComponent subclass, so it is described
// in ClassDB, and ClassDB is what this listens to -- every IComponent that registers, whenever it registers, becomes a
// component here without anyone naming its C++ type. That is what lets a plugin contribute a component type the engine
// was never compiled against, and why no code is generated for component registration any more.
//
// flecs appears nowhere in this header. The world itself is held behind a pointer to an incomplete type, and every
// operation that needs flecs is defined in world.cpp.
//
// abstract: reflected so its API is reachable by name, but never built through ClassDB -- a second World would be a
// second ECS, and WorldSim owns the one.
class FEATHER_API World : public Reflected {
	FCLASS(abstract);

	struct Impl;
	std::unique_ptr<Impl> _impl;

	// Component id per registered class name. Also the record of what has already been registered, since flecs would
	// otherwise define the same name twice.
	std::unordered_map<StaticString, EntityId> _components;

	Delegate<std::string_view>::id_t _component_delegate = -1;
	Delegate<std::string_view>::id_t _module_delegate = -1;

	// Modules already imported, by class name. WorldSim both sweeps ClassDB and subscribes to it, so the same module
	// can be reached twice.
	std::unordered_map<StaticString, EntityId> _modules;

	// Opens a module scope named after `class_name`, returning the scope to restore.
	// Shared by import_module<T> so the template stays thin -- and flecs-free.
	EntityId _begin_module(StaticString class_name, EntityId& out_module);
	void _end_module(EntityId previous_scope);

	// Imports one module by class name, through the static hook codegen gives every EcsModule subclass.
	void _import_module_by_name(StaticString class_name);

	// The half of register_component_type<T> that actually talks to flecs.
	EntityId _register_component_raw(StaticString name, const ValueTypeOps& ops);

public:
	World();
	~World() override;

	// Neither copyable nor movable: the world subscribes to ClassDB with a delegate capturing `this`, so a moved-from
	// World would leave that subscription pointing at a corpse. WorldSim owns the one instance in place.
	World(const World&) = delete;
	World& operator=(const World&) = delete;
	World(World&&) = delete;
	World& operator=(World&&) = delete;

	// A name kept alive for the life of the process. flecs keeps the `const char*` it is handed and StaticString is a
	// non-owning view, so a name that came from a std::string needs somewhere permanent to point.
	static StaticString _intern(std::string_view name);

	// ---- Simulation --------------------------------------------------------

	[[method]] bool progress(double delta = 0.0);

	// Creates a timer entity ticking every `interval` seconds, for a system that runs on a fixed step rather than
	// every frame. Hand the result to SystemBuilder::tick_source.
	[[nodiscard]] EntityId create_timer(double interval);

	// Time elapsed in the frame currently being simulated. A system reads it from here rather than from its iterator,
	// so an each() callback does not have to take one just to know the timestep.
	[[nodiscard]] float delta_time() const;

	// The flecs REST explorer, for debug builds. Named for what it does rather than for the component it adds.
	void enable_rest_api();

	// ---- Entities ----------------------------------------------------------

	[[nodiscard]] Entity create_entity(const std::string& name = "");
	[[nodiscard]] Entity create_entity(const Entity& parent, const std::string& name = "");
	[[nodiscard]] Entity entity(EntityId id);
	[[nodiscard]] Entity prefab(const std::string& name);
	[[nodiscard]] Entity lookup(const std::string& name);
	void destroy_entity(EntityId id) const;
	[[nodiscard]] bool is_valid(EntityId id) const;

	// An entity's own attributes. Entity is a handle with no storage of its own, so everything it reports goes
	// through here rather than through a flecs handle it would have to rebuild on every call.
	[[nodiscard]] std::string get_entity_name(EntityId id) const;
	void set_entity_name(EntityId id, const std::string& name) const;
	[[nodiscard]] EntityId get_parent(EntityId id) const;
	void set_parent(EntityId child, EntityId parent) const;
	// Makes `entity` an instance of `prefab`, inheriting whatever the prefab declares.
	void set_prefab(EntityId entity, EntityId prefab) const;
	[[nodiscard]] bool is_instance_of(EntityId entity, EntityId prefab) const;

	// ---- Component types, by reflection ------------------------------------

	// Registers `class_name` with the ECS, using the storage its ClassInfo describes. Idempotent: a name already
	// registered returns the id it got the first time. Returns a null id when the class is unknown, is not a value
	// type, or has no storage to give.
	EntityId register_component(StaticString class_name);

	// The id a class name was registered under, or a null id.
	[[nodiscard]] EntityId find_component(StaticString class_name) const;

	// One field of a component being described at runtime.
	struct FieldDesc {
		std::string name;
		VariantType type = VariantType::NIL;
	};

	// Registers a component whose fields are described at runtime rather than by a C++ type, laying its storage out
	// here. Returns a null id with *error set on a bad description -- a normal outcome, since the description comes
	// from outside the engine. Field types are limited to the trivially-copyable Variant types, because the storage
	// has to be memcpy-able.
	EntityId register_component(
			const std::string& name,
			std::span<const FieldDesc> fields,
			std::string* error = nullptr
	);

	// Registers a component for a C++ type that is not reflected, and so can never arrive through ClassDB -- the
	// third-party math types are the reason this exists (see ecs/math_module.cpp).
	template <typename T>
	EntityId register_component_type(const char* name);

	// ---- Components on entities, by name -----------------------------------

	bool add_component(EntityId entity, StaticString class_name);
	[[nodiscard]] bool has_component(EntityId entity, StaticString class_name) const;
	void remove_component(EntityId entity, StaticString class_name);
	void remove_all(StaticString class_name);

	// Raw storage for a component instance, or nullptr. `mutable_component_data` also marks it changed, which is what
	// a caller writing through it needs.
	[[nodiscard]] const void* component_data(EntityId entity, StaticString class_name) const;
	[[nodiscard]] void* mutable_component_data(EntityId entity, StaticString class_name);

	// Named property access lives on Component (ecs/component.h), which caches the ClassInfo and the component id
	// rather than re-resolving both on every field touch. There is deliberately no by-name property pair here.

	// Copies `size` bytes into the component's storage, adding it if absent. What Entity::set<T> reduces to.
	bool _set_component_raw(EntityId entity, StaticString class_name, const void* value, size_t size);

	// ---- Modules -----------------------------------------------------------

	// Constructs an EcsModule subclass with this world, once, under a module scope named after it so everything it
	// declares is namespaced the way flecs expects. Feather's own import rather than flecs's, because a module is
	// handed this World, not the flecs one.
	template <typename T>
	EntityId import_module();

	[[nodiscard]] bool is_module_imported(StaticString class_name) const;

	// Imports every EcsModule subclass ClassDB knows, then keeps listening so one that arrives later -- from a project
	// DLL loaded after startup -- is imported too. The same arrangement components get, and for the same reason.
	//
	// Deliberately not done in the constructor: registering a class fires the delegate immediately, so a module would
	// import into a world that has no content yet. The owner calls this once it is ready (see WorldSim::init).
	void import_modules();

	// ---- Systems and queries ----------------------------------------------
	// Non-template on purpose: the builders in system_builder.h and query.h describe what they want as plain data and
	// hand it over here, which is what keeps flecs out of their headers.

	EntityId _register_system(SystemDesc&& desc);

	// Returns an opaque ecs_query_t*, owned by the caller -- Query's destructor hands it back to _destroy_query.
	[[nodiscard]] void* _create_query(QueryDesc&& desc);
	void _destroy_query(void* query) const;

	// Runs `callback` once per matching batch. Synchronous, so `ctx` need only outlive the call.
	void _query_each(void* query, SystemCallback callback, void* ctx);

	// ---- Reflected surface -------------------------------------------------
	// Names, never ids: an EntityId is not Variant-marshalable, so anything taking one stays C++-only and a script
	// reaches entities through Entity instead.

	[[method]] bool has_component_type(std::string class_name) const;
	[[method]] bool register_component_type_by_name(std::string class_name);
	[[method]] bool has_module(std::string class_name) const;
	[[method]] int get_component_type_count() const;
};

template <typename T>
EntityId World::register_component_type(const char* name) {
	const StaticString interned = _intern(name);
	if (const EntityId existing = find_component(interned); !existing.is_null()) {
		return existing;
	}
	return _register_component_raw(interned, make_value_type_ops<T>());
}

template <typename T>
EntityId World::import_module() {
	const StaticString name = T::get_class_static();
	if (auto it = _modules.find(name); it != _modules.end()) {
		return it->second;
	}

	EntityId module_entity;
	const EntityId previous = _begin_module(name, module_entity);
	// Recorded before constructing: a module that imports another one during its own constructor must not start this
	// one a second time.
	_modules[name] = module_entity;
	{
		T instance(*this);
		(void)instance;
	}
	_end_module(previous);
	return module_entity;
}

} //namespace feather
