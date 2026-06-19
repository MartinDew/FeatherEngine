#include "ecs_feature.h"

#include "main/world_sim.h"

namespace feather {

WorldSim* EcsFeature::_get_world_sim() {
	return WorldSim::get();
}

} // namespace feather