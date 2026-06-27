#pragma once

#include "simulation.h"
#include <world/world.h>
#include <framework/reflection_macros.h>

namespace feather {

class WorldSim final : public Simulation {
	FCLASS_SINGLETON(WorldSim, Simulation);

	World _world;

protected:
	static void _bind_members();

public:
	WorldSim();
	~WorldSim() override;

	void update(double delta) override;

	[[nodiscard]] World& get_world() { return _world; }
};

} //namespace feather