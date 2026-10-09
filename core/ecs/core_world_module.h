#pragma once

#include "ecs_module.h"
#include "world.h"

#include <framework/export_defs.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "core_world_module.gen.h"
#endif

namespace feather {

class FEATHER_API CoreEcsModule final : public EcsModule {
	FCLASS();

public:
	CoreEcsModule() = default;

	void on_import(World& world) override;
};

} //namespace feather
