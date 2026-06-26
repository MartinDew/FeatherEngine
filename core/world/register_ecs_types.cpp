#include "register_ecs_types.h"

#include "core_ecs_module.h"
#include "math_ecs_module.h"
#include "rendering_ecs_module.h"

namespace feather {

void register_ecs_types(World& world) {
	world.import_module<MathEcsModule>();
	world.import_module<CoreWorldFeature>();
	world.import_module<RenderingEcsModule>();
}

} //namespace feather