#pragma once

#include "components/component_interface.h"
#include "components/light.h"
#include "ecs_defs.h"
#include "ecs_module.h"
#include "system_iterator.h"
#include "world.h"

#include <math/transform.h>

#include <framework/export_defs.h>

// WorldSim must be a complete type here (not just the forward decl from ecs_module.h): the generated
// register_ecs_types.gen.cpp binds _import_module via ClassDB::bind_static_method, which needs
// VariantCompatible<WorldSim*> to resolve std::is_base_of_v<Reflected, WorldSim>.
#include <main/world_sim.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "rendering_world_module.gen.h"
#endif

namespace feather {

class Mesh;
class Material;

// Constructors rather than aggregate initialization: a component derives from IComponent, and a base subobject would
// have to be spelled in a braced list.
struct MeshInstance : IComponent {
	FSTRUCT();

	std::shared_ptr<Mesh> mesh;

	MeshInstance() = default;
	explicit MeshInstance(std::shared_ptr<Mesh> mesh) : mesh(std::move(mesh)) {}
};

struct MaterialInstance : IComponent {
	FSTRUCT();

	std::shared_ptr<Material> material; // todo: multiple materials

	MaterialInstance() = default;
	explicit MaterialInstance(std::shared_ptr<Material> material) : material(std::move(material)) {}
};

class FEATHER_API RenderingEcsModule : public EcsModule {
	FCLASS(EcsModule);

	// [[system]] says these run against the world rather than against an instance, and holds them to it: static, and
	// members of the module that declares them. Codegen rejects the attribute anywhere else.
	[[system]] static void _begin_render_scene(SystemIterator& it);
	[[system]] static void _update_meshes(Entity entity, Transform transform, MeshInstance& mesh, MaterialInstance* material);
	[[system]] static void _fill_lights(Entity entity, const Light& light);
	[[system]] static void _commit_render_scene(SystemIterator& it);

public:
	RenderingEcsModule() = default;
	RenderingEcsModule(World& world);
};

} //namespace feather
