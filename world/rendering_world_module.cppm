module;

// WorldSim must be a complete type here (not just the forward decl from
// ecs_module.h): the generated register_world_types.gen.cpp binds
// _load_module via ClassDB::bind_static_method, which needs
// VariantCompatible<WorldSim*> to resolve std::is_base_of_v<Reflected, WorldSim>.

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "rendering_world_module.gen.h"
#endif

export module feather.world:rendering_world_module;

import std;
import feather.framework;
import :ecs_defs;
import :ecs_module;
import :world_sim;
import feather.reflection;
import feather.rendering;

export namespace feather {

struct MeshInstance {
	FSTRUCT(Component);

	std::shared_ptr<Mesh> mesh;
};

struct MaterialInstance {
	FSTRUCT(Component);

	std::shared_ptr<Material> material; // todo: multiple materials
};

class FEATHER_WORLD_API RenderingWorldModule : public EcsModule {
	FCLASS(EcsModule);

public:
	RenderingWorldModule() = default;
	RenderingWorldModule(World world);
};

} //namespace feather
