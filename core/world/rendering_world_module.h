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

class RenderingWorldModule : public EcsFeature {
	FCLASS(RenderingWorldModule, EcsFeature);

protected:
	static void _bind_members();

	static void _load_module(WorldSim* sim);

public:
	RenderingWorldModule() = default;
	RenderingWorldModule(World world);
};

} //namespace feather