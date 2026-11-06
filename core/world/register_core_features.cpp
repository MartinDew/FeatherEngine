module;

module feather.core;

namespace feather {

void register_core_components(Ecs& ecs) {
	register_world_components(ecs);
	register_math_components(ecs);
}

} //namespace feather
