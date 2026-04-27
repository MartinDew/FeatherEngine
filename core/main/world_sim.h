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
	Entity _current_scene;

	std::vector<Entity> _scenes;

	std::map<StaticString, Entity> _prefabs;

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
	Entity create_scene() const;

	[[nodiscard]]
	Entity add_entity(const Entity& scene) const;

	// get low level world impl
	[[nodiscard]] World* get_world() { return &_world; }

	void add_to_scene(Entity entity) const;

	template <class... T>
	Ecs::system_builder<T...>& execute_fixed(Ecs::system_builder<T...>& system) {
		return system.tick_source(fixed_tick);
	}
};

} //namespace feather