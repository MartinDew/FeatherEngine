module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "math_module.gen.h"
#endif

export module feather.core:world.math_module;

import :world.ecs_defs;
import :world.ecs_module;
import :main.class_db;

export namespace feather {

class FEATHER_API MathWorldModule final : public EcsModule {
	FCLASS(EcsModule);

public:
	MathWorldModule();
	MathWorldModule(World& world);
};

} //namespace feather
