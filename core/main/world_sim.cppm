module;

#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#include <framework/singleton_helpers.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "world_sim.gen.h"
#endif

export module feather.core:main.world_sim;

import std;
import :framework.static_string;
import :main.simulation;
import :framework.delegate;
import :world.components.scene;
import :world.ecs_defs;
import :world.ecs;
import :framework.assert;
import :main.class_db;

export namespace feather {

class FEATHER_API WorldSim final : public Simulation {
	FCLASS(singleton);

	Ecs _ecs;
	Entity _scene_prefab;
	Entity _current_scene;

	std::vector<Entity> _scenes;

	ClassDB::subclass_delegate_t::id_t _subclass_delegate_id = -1;

	bool _is_in_scene(Entity e, Entity scene) const;

	template <class... TComps, class TFunc>
	void _iterate_tree(Entity e, TFunc func) {
		// Todo
	}

protected:
	template <ClassDbNamed T>
	void _import_object() {
		_ecs.import_object<T>();
	}

public:
	const EntityId fixed_tick;

	WorldSim();
	~WorldSim() override;

	void init() override;

	void update(double delta) override;

	[[nodiscard]] Entity& get_current_scene() { return _current_scene; }

	[[nodiscard]]
	Entity create_scene(const std::string& name);

	[[nodiscard]]
	Entity create_entity(const std::string& name = "");
	[[nodiscard]]
	Entity create_entity(const Entity& parent_entity, const std::string& name = "");

	[[nodiscard]] Ecs* get_ecs() { return &_ecs; }

	void add_to_scene(Entity entity);

	template <class... T>
	SystemBuilder<T...>& execute_fixed(SystemBuilder<T...>& system) {
		return system.tick_source(fixed_tick);
	}

	void set_active_scene(Entity scene);

	// Build a query scoped to the active scene
	template <class... TComps>
	auto scene_query() {
		return _ecs.query<TComps...>().template with<ActiveScene>(TraverseFlag::Up).build();
	}

	template <class... TComps>
	auto scene_system() {

	};
};

} //namespace feather
