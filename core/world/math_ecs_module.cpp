#include "math_ecs_module.h"

#include "math/transform.h"

namespace feather {

void MathEcsModule::_bind_members() {
}

MathEcsModule::MathEcsModule() = default;

MathEcsModule::MathEcsModule(World world) {
	world.component<Transform>("Transform");
	world.component<Vector3>("Vector3");
	world.component<Matrix>("Matrix");
	world.component<Color>("Color");
}

} //namespace feather