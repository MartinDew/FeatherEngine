#pragma once
#include "ecs_defs.h"
#include "world_module.h"

namespace feather {

struct MeshInstance {
	RID MeshId;
};

struct MaterialInstance {
	RID MaterialId; // todo: multiple materials
};

class RenderingWorldModule : public WorldModule {
	FCLASS(RenderingWorldModule, WorldModule);

protected:
	static void _bind_methods();

public:
	RenderingWorldModule(World* world);
};

} //namespace feather