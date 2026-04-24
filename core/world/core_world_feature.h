#pragma once
#include "ecs_defs.h"
#include "ecs_feature.h"

namespace feather {

class CoreWorldFeature : public EcsFeature {
	FCLASS(CoreWorldFeature, EcsFeature);

protected:
	static void _bind_members();
	CoreWorldFeature() = default;
	CoreWorldFeature(World world);
};

} //namespace feather
