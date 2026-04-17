#include "world_sim.h"

#include <world/components/scene.h>

namespace feather {

static Entity prefab_new_scene;

void WorldSim::_bind_members() {
}

WorldSim::WorldSim() {
	prefab_new_scene = _world.prefab("new_scene").add<Scene>();
	_scenes.push_back(create_scene());
}

WorldSim::~WorldSim() {
}

void WorldSim::init() {
}

void WorldSim::fixed_update(double delta) {
}

void WorldSim::update() {
}

Entity WorldSim::create_scene() const {
	return _world.entity().is_a(prefab_new_scene);
}

Entity WorldSim::add_entity(const Entity& scene) const {
	return _world.entity().add(scene);
}

} //namespace feather