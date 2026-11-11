module;

module feather.world;

namespace feather {

void register_core_components(World& world) {
	register_math_components(world);
	register_rendering_components(world);
	register_world_components(world);
}

} //namespace feather
