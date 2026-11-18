module;

#include <framework/reflection_macros.h>

#include <flecs.h>
#include <flecs/addons/cpp/world.hpp>

#include <framework/singleton_helpers.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "world_sim.gen.h"
#endif

export module feather.world:world_sim;

import std;
import feather.framework;
import :simulation;
import :components.scene;
import :ecs_defs;
import :world_ecs;
import feather.reflection;

export namespace feather {

class FEATHER_WORLD_API WorldSim final : public Simulation {
	FCLASS(singleton);

	World _world;
	Entity _scene_prefab;
	Entity _current_scene;

	std::vector<Entity> _scenes;

	ClassDB::subclass_delegate_t::id_t _subclass_delegate_id = -1;
	ClassDB::subclass_delegate_t::id_t _class_delegate_id = -1;

	// In world_sim.h, private section:
	bool _is_in_scene(flecs::entity e, Entity scene) const;

protected:
	template <std::derived_from<class EcsModule> T>
	void _import_feature() {
		_world.import<T>();
	}

public:
	const EcsTimer fixed_tick;

	WorldSim();
	~WorldSim() override;

	void init() override;

	void update(double delta) override;

	[[nodiscard]] inline Entity& get_current_scene() { return _current_scene; }

	[[nodiscard]]
	Entity create_scene(const std::string& name) const;

	// The world this simulation owns; the one scripts reach.
	[[method]] [[nodiscard]] inline World* get_world() { return &_world; }

	void add_to_scene(Entity entity) const;

	template <class... T>
	flecs::system_builder<T...>& execute_fixed(flecs::system_builder<T...>& system) {
		return system.tick_source(fixed_tick);
	}

	void set_active_scene(Entity scene);

	// Build a query scoped to the active scene
	template <class... TComps>
	auto scene_query() {
		return _world.query_builder<TComps...>().template with<ActiveScene>().up(flecs::ChildOf).build();
	}
};

} //namespace feather
