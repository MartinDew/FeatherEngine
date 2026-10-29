module;

module feather.core;

namespace feather {

WorldSim* EcsObject::_get_world_sim() {
	return WorldSim::get();
}

} // namespace feather
