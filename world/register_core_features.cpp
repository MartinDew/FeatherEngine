module;

module feather.world;

namespace feather {

void register_core_components(World& world) {
	const auto reg = [&world]<class T>() { world.register_component<T>(); };
	register_rendering_components(reg);
	register_world_components(reg);
}

} //namespace feather
