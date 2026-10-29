module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "rendering_ecs_object.gen.h"
#endif

export module feather.core:world.rendering_ecs_object;

import std;
import :framework.static_string;
import :world.ecs;
import :world.ecs_object;
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

class FEATHER_API RenderingEcsObject : public EcsObject {
	FCLASS(EcsObject);

public:
	RenderingEcsObject() = default;

	void on_import(Ecs& ecs) override;
};

} //namespace feather
