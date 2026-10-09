module;

#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "ecs_module.gen.h"
#endif

export module feather.core:world.ecs_module;

import :framework.reflected;
import :main.class_db;

export namespace feather {

class WorldSim;

class FEATHER_API EcsModule : public Reflected {
	FCLASS();

protected:
	EcsModule() = default;

	static WorldSim* _get_world_sim();
};

} //namespace feather
