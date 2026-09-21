#pragma once

#include "ecs_module.h"
#include "world.h"

#include <framework/export_defs.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "math_module.gen.h"
#endif

namespace feather {

class FEATHER_API MathEcsModule final : public EcsModule {
	FCLASS();

public:
	MathEcsModule() = default;

	void on_import(World& world) override;
};

} //namespace feather
