module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs_module.gen.h"
#endif

export module feather.world:ecs_module;

import std;
import feather.framework;
import feather.reflection;

export namespace feather {

class WorldSim;

class FEATHER_WORLD_API EcsModule : public Reflected {
	FCLASS();

protected:
	EcsModule() = default;

	static WorldSim* _get_world_sim();
};

} //namespace feather
