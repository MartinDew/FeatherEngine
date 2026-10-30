module feather.core;

namespace feather {

void MathEcsObject::on_import(Ecs& ecs) {
	// Transform is a Component registered up front; these are reflected value types usable as components too.
	ecs.component<Vector3>();
	ecs.component<Matrix>();
	ecs.component<Color>();
}

} //namespace feather
