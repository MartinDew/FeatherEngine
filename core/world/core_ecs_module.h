#pragma once
#include "world.h"
#include "ecs_module.h"

namespace feather {

class CoreWorldFeature : public EcsModule {
	FCLASS(CoreWorldFeature, EcsModule);

protected:
	static void _bind_members();

public:
	CoreWorldFeature() = default;
	CoreWorldFeature(World world);
};

} //namespace feather
