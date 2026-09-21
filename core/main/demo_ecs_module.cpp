#include "demo_ecs_module.h"

namespace feather {

void DemoEcsModule::_spin(Entity entity, const MeshInstance& mesh, Transform& transform) {
	const auto delta = static_cast<real_t>(entity.get_world()->delta_time());
	transform.rotation = transform.rotation * Quaternion::create_from_yaw_pitch_roll(Vector3 { 0, delta, 0 });
}

DemoEcsModule::DemoEcsModule(World& world) {
	system<const MeshInstance, Transform>(world, "Spin").with<Move>().phase(SystemPhase::OnUpdate).each(&_spin);
}

} //namespace feather
