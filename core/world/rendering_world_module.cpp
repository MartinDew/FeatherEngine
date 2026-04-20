#include "rendering_world_module.h"

#include "main/world_sim.h"

namespace feather {

void RenderingWorldModule::_bind_methods() { WorldSim::get() -> }

RenderingWorldModule::RenderingWorldModule(World* world) {
	world->module<Type>();
}

} //namespace feather