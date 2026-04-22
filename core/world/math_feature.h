#pragma once
#include "ecs_defs.h"
#include "ecs_feature.h"

namespace feather {

class MathWorldFeature final : public EcsFeature {
	FCLASS(MathWorldFeature, EcsFeature);

protected:
	static void _bind_members();

public:
	MathWorldFeature();
	MathWorldFeature(World& world);
};

} //namespace feather