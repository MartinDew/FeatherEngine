#pragma once
#include "ecs_defs.h"
#include "ecs_module.h"

namespace feather {

class MathEcsModule final : public EcsModule {
	FCLASS(MathEcsModule, EcsModule);

protected:
	static void _bind_members();

public:
	MathEcsModule();
	MathEcsModule(World& world);
};

} //namespace feather