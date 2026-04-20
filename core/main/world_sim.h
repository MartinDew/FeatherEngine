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

	Entity _fixed_update_phase;
	Entity _fixed_update_pipeline;

	std::vector<Entity> _scenes;

	std::map<StaticString, Entity> _prefabs;

protected:
	static void _bind_members();

	template <class T>
	void _import_feature() {
		_world.import <T>();
	}

public:
	WorldSim();
	~WorldSim() override;

	void init() override;
	void pre_update(double delta) override;
	void fixed_update(double delta) override;
	void update(double delta) override;

	[[nodiscard]] Entity get_fixed_update_phase() const { return _fixed_update_phase; }

	[[nodiscard]]
	Entity create_scene() const;

	[[nodiscard]]
	Entity add_entity(const Entity& scene) const;

	// get low level world impl
	World* get_world() { return &_world; }
};

} //namespace feather