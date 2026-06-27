#include "ecs_module.h"

#include <main/world_sim.h>

namespace feather {

World* EcsModule::_get_world() {
	return &WorldSim::get()->get_world();
}

} // namespace feather
