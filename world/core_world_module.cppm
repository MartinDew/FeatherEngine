module;

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "core_world_module.gen.h"
#endif

export module feather.world:core_world_module;

import std;
import feather.framework;
import :ecs_defs;
import :ecs_module;
import feather.reflection;

export namespace feather {

class FEATHER_WORLD_API CoreWorldModule : public EcsModule {
	FCLASS(EcsModule);

public:
	CoreWorldModule() = default;
	CoreWorldModule(World world);
};

} //namespace feather
