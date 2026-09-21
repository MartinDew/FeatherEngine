#pragma once

#include <ecs/components/component_interface.h>
#include <ecs/ecs_module.h>
#include <ecs/entity.h>
#include <ecs/rendering_world_module.h>
#include <ecs/world.h>
#include <framework/export_defs.h>
#include <math/transform.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "demo_ecs_module.gen.h"
#endif

namespace feather {

// Marks an entity the demo scene spins. Empty, so it costs no storage and registers as a tag.
struct Move : IComponent {
	FSTRUCT();
};

// The demo scene's systems. A system must belong to a module, so the demo's spin system lives here rather than as a
// loose function or lambda registered directly against the world.
//
// Imported like any other module, in every build, but matches nothing until something carries Move, which only
// Engine::_setup_demo_scene ever adds.
class FEATHER_API DemoEcsModule final : public EcsModule {
	FCLASS(EcsModule);

	[[system]] static void _spin(Entity entity, const MeshInstance& mesh, Transform& transform);

public:
	DemoEcsModule() = default;
	DemoEcsModule(World& world);
};

} //namespace feather
