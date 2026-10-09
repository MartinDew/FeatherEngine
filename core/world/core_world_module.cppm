module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "core_world_module.gen.h"
#endif

export module feather.core:world.core_world_module;

import :world.ecs_defs;
import :world.ecs_module;
import :main.class_db;

export namespace feather {

class FEATHER_API CoreWorldModule : public EcsModule {
	FCLASS(EcsModule);

public:
	CoreWorldModule() = default;
	CoreWorldModule(World world);
};

} //namespace feather
