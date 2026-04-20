#include "rendering_world_module.h"

#include "components/light.h"
#include "main/world_sim.h"

namespace feather {

void RenderingWorldModule::_bind_members() {
	ClassDB::bind_static_method(&Type::_load_module, "_import_module");
}

void RenderingWorldModule::_load_module(WorldSim* sim) {
	sim->get_world()->import <Type>();
}

RenderingWorldModule::RenderingWorldModule(World world) {
	std::println("importing module {} ", get_class_static());
	world.module<Type>();

	world.component<MeshInstance>();
	world.component<MaterialInstance>();
	world.component<Light>();
}

} //namespace feather