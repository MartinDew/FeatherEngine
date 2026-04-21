#pragma once
#include "ecs_defs.h"
#include "ecs_feature.h"

namespace feather {

struct MeshInstance {
	RID MeshId;
};

struct MaterialInstance {
	RID MaterialId; // todo: multiple materials
};

class RenderingWorldFeature : public EcsFeature {
	FCLASS(RenderingWorldFeature, EcsFeature);

protected:
	static void _bind_members();

	static void _load_module(WorldSim* sim);

public:
	RenderingWorldFeature() = default;
	RenderingWorldFeature(World world);
};

} //namespace feather