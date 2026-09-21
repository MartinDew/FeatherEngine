#include "register_core_features.h"

#include "register_ecs_types.gen.h"
#include <math/register_math_types.gen.h>

namespace feather {

void register_core_components(World& world) {
	register_ecs_components(world);
	register_math_components(world);
}

} //namespace feather