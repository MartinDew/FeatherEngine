#include "register_core_features.h"

#include "math_feature.h"
#include "rendering_world_feature.h"

namespace feather {

void register_core_ecs_features(World& world) {
	world.import <MathWorldFeature>();
	world.import <RenderingWorldFeature>();
}

} //namespace feather