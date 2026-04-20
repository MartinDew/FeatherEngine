#pragma once

#include "simulation.h"
#include <framework/reflection_macros.h>

#include <world/ecs_defs.h>

#include <flecs.h>
#include <flecs/addons/cpp/world.hpp>

namespace feather {

class WorldSim final : public Simulation {
	FCLASS_SINGLETON(WorldSim, Simulation);

	flecs::world _world;

	flecs::entity _fixed_update_phase;
	flecs::entity _fixed_update_pipeline;

	std::vector<Entity> _scenes;

	std::map<StaticString, Entity> _prefabs;

protected:
	static void _bind_members();

public:
	WorldSim();
	~WorldSim() override;

	void init() override;
	void pre_update(double delta) override;
	void fixed_update(double delta) override;
	void update(double delta) override;

	flecs::entity get_fixed_update_phase() const { return _fixed_update_phase; }

	[[nodiscard]]
	Entity create_scene() const;

	[[nodiscard]]
	Entity add_entity(const Entity& scene) const;
};

} //namespace feather