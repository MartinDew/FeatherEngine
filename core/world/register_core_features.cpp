module;

module feather.core;

namespace feather {

void register_core_components(World& world) {
	register_world_components(world);
	register_math_components(world);
}

} //namespace feather
