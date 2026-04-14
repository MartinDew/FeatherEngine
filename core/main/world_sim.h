#pragma once

#include "simulation.h"

#include <world/ecs_defs.h>

#include <flecs.h>
#include <flecs/addons/cpp/world.hpp>

namespace feather {

class WorldSim final : public Simulation {
	FCLASS(WorldSim, Simulation);

	flecs::world _world;

	std::vector<Entity> _scenes;

public:
	WorldSim();
	~WorldSim() override;

	void init() override;
	void fixed_update(double delta) override;
	void update() override;

	Entity create_scene() const;

	Entity get_root_scene() const;
};

} //namespace feather