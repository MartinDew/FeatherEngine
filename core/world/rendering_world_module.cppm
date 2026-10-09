module;

#include <framework/export_defs.h>

// WorldSim must be a complete type here (not just the forward decl from
// ecs_module.h): the generated register_world_types.gen.cpp binds
// _load_module via ClassDB::bind_static_method, which needs
// VariantCompatible<WorldSim*> to resolve std::is_base_of_v<Reflected, WorldSim>.

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "rendering_world_module.gen.h"
#endif

export module feather.core:world.rendering_world_module;

import :world.ecs_defs;
import :world.ecs_module;
import :main.world_sim;
import :main.class_db;

export namespace feather {

class Mesh;
class Material;

struct MeshInstance {
	FSTRUCT(Component);

	std::shared_ptr<Mesh> mesh;
};

struct MaterialInstance {
	FSTRUCT(Component);

	std::shared_ptr<Material> material; // todo: multiple materials
};

class FEATHER_API RenderingWorldModule : public EcsModule {
	FCLASS(EcsModule);

public:
	RenderingWorldModule() = default;
	RenderingWorldModule(World world);
};

} //namespace feather
