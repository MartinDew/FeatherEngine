#pragma once

#include "components/component_interface.h"
#include "components/light.h"
#include "ecs_defs.h"
#include "ecs_module.h"
#include "system_iterator.h"
#include "world.h"

#include <math/transform.h>

#include <framework/export_defs.h>

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

class FEATHER_API RenderingEcsModule final : public EcsModule {
	FCLASS();

	// Static because a system outlives the module object, which exists only for the duration of on_import.
	static void _begin_render_scene(SystemIterator& it);
	static void _update_meshes(Entity entity, Transform transform, MeshInstance& mesh, MaterialInstance* material);
	static void _fill_lights(Entity entity, const Light& light);
	static void _commit_render_scene(SystemIterator& it);

public:
	RenderingEcsModule() = default;

	void on_import(World& world) override;
};

} //namespace feather
