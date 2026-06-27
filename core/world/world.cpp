#include "world.h"
#include "components/scene.h"
#include <framework/assert.h>

namespace feather {

Entity World::entity(const char* name) {
	return name ? _flecs.entity(name) : _flecs.entity();
}

Entity World::entity(Entity scene, const char* name) {
	return _flecs.entity(name).child_of(scene);
}

void World::progress(double delta) {
	_flecs.progress(delta);
}

void World::init_scenes() {
	_scene_prefab = _flecs.prefab("Scene");
}

Entity World::create_scene(std::string name) {
	Scene s { { name } };
	return _flecs.entity(name.c_str()).is_a(_scene_prefab).set<Scene>(s);
}

Entity World::create_entity(std::string name) const {
	return _flecs.entity(name.c_str());
}

Entity World::create_entity(const Entity& parent, std::string name) const {
	return _flecs.entity(name.c_str()).child_of(parent);
}

void World::add_to_scene(Entity entity) const {
	entity.child_of(_current_scene);
}

void World::set_active_scene(Entity scene) {
	fassert(scene.is_a(_scene_prefab), "Given scene isn't a scene instance");
	_flecs.remove_all<ActiveScene>();
	scene.add<ActiveScene>();
	_current_scene = scene;
}

void World::set_fixed_tick_interval(double interval) {
	_fixed_tick = _flecs.timer().interval(static_cast<float>(interval));
}

} // namespace feather