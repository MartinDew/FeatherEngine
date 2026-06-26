#pragma once
#include "world.h"
#include "ecs_module.h"

namespace feather {

class Mesh;
class Material;
class WorldSim;

struct MeshInstance {
	std::shared_ptr<Mesh> mesh;
};

struct MaterialInstance {
	std::shared_ptr<Material> material; // todo: multiple materials
};

class RenderingEcsModule : public EcsModule {
	FCLASS(RenderingEcsModule, EcsModule);

protected:
	static void _bind_members();

	static void _load_module(WorldSim* sim);

public:
	RenderingEcsModule() = default;
	RenderingEcsModule(World world);
};

} //namespace feather
