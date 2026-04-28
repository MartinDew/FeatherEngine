#pragma once

#include "simulation.h"
#include <framework/reflection_macros.h>

#include <world/ecs_defs.h>

#include <flecs.h>
#include <flecs/addons/cpp/world.hpp>

namespace feather {

class WorldSim final : public Simulation {
	FCLASS_SINGLETON(WorldSim, Simulation);

	World _world;
	Entity _scene_prefab;
	Entity _current_scene;
	Ecs::observer _scene_observer;

	std::vector<Entity> _scenes;

	// In world_sim.h, private section:
	void _tag_descendants_in_scene(Entity parent) const;
	bool _is_in_scene(flecs::entity e, Entity scene) const;

protected:
	static void _bind_members();

	template <std::derived_from<class EcsFeature> T>
	void _import_feature() {
		_world.import <T>();
	}

public:
	const EcsTimer fixed_tick;

	WorldSim();
	~WorldSim() override;

	void update(double delta) override;

	[[nodiscard]] Entity& get_current_scene() { return _current_scene; }

	[[nodiscard]]
	Entity create_scene(std::string name) const;

	[[nodiscard]]
	Entity add_entity(std::string name = "") const;
	[[nodiscard]]
	Entity add_entity(const Entity& scene, std::string name = "") const;

	// get low level world impl
	[[nodiscard]] World* get_world() { return &_world; }

	void add_to_scene(Entity entity) const;

	template <class... T>
	Ecs::system_builder<T...>& execute_fixed(Ecs::system_builder<T...>& system) {
		return system.tick_source(fixed_tick);
	}

	void set_active_scene(Entity scene);
};

} //namespace feather