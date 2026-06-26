#include "core_ecs_module.h"

#include "components/scene.h"

#include <main/world_sim.h>

namespace feather {

void CoreWorldFeature::_bind_members() {
}

CoreWorldFeature::CoreWorldFeature(World world) {
	world.component<Scene>("Scene");
	world.component<ActiveScene>("ActiveScene");
	auto in_scene = world.component<InScene>();
}

} //namespace feather