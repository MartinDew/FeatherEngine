#pragma once

#include "world.h"

#include <framework/export_defs.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs_module.gen.h"
#endif

namespace feather {

class WorldSim;

// A feature's worth of ECS content: the systems it runs, and any component type with no C++ class to declare it.
// Found through ClassDB the way a component is, built by it, and asked once for what it owns through on_import.
class FEATHER_API EcsModule : public Reflected {
	FCLASS();

protected:
	EcsModule() = default;

	static WorldSim* _get_world_sim();

public:
	// Declares what the module owns. Called once per world, inside a module scope named after the class, so everything
	// declared here is namespaced the way flecs expects.
	virtual void on_import(World& world) = 0;
};

} //namespace feather
