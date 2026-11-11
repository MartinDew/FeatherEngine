module;

module feather.world;

namespace feather {

WorldSim* EcsModule::_get_world_sim() {
	return WorldSim::get();
}

} // namespace feather
