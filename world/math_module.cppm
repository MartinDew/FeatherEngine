module;

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "math_module.gen.h"
#endif

export module feather.world:math_module;

import std;
import feather.framework;
import :ecs_defs;
import :ecs_module;
import feather.reflection;

export namespace feather {

class FEATHER_WORLD_API MathWorldModule final : public EcsModule {
	FCLASS(EcsModule);

public:
	MathWorldModule();
	MathWorldModule(World& world);
};

} //namespace feather
