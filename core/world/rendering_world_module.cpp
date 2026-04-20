#include "rendering_world_module.h"

#include "components/light.h"
#include "main/world_sim.h"

namespace feather {

void RenderingWorldModule::_bind_members() {
}

RenderingWorldModule::RenderingWorldModule(World* world) {
	world->module<Type>();

	world->component<MeshInstance>();
	world->component<MaterialInstance>();
	world->component<Light>();
}

} //namespace feather