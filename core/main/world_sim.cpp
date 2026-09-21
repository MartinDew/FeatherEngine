#include "world_sim.h"

#include "engine.h"

#include <ecs/components/scene.h>
#include <framework/static_string.hpp>

namespace feather {

FSINGLETON_INSTANCE(WorldSim);

WorldSim::WorldSim() : fixed_tick { _world.create_timer(Engine::simulation_time) } {
	FSINGLETON_CONSTRUCT_INSTANCE()
#if BETA
	_world.enable_rest_api();
#endif

	// A World knows nothing of ClassDB on its own. This is the world the engine simulates, so it gets the reflected
	// component types, and keeps getting them as a project DLL registers more.
	_world.register_classdb_components();
}

void WorldSim::init() {
	_scene_prefab = _world.prefab("Scene");
	auto scene = create_scene("new scene");
	fassert(scene.is_valid());
	set_active_scene(scene);

	// Only now: a module's systems need the scene content to run against, and this is after index_project(), so a
	// module from a project DLL is picked up by the same sweep as core's.
	_world.import_classdb_modules();
}

void WorldSim::update(double delta) {
	_world.progress();
}

Entity WorldSim::create_scene(const std::string& name) {
	Scene s { StaticString(name) };
	Entity scene = _world.create_entity(name);
	return scene.is_a(_scene_prefab).set<Scene>(s);
}

void WorldSim::add_to_scene(Entity entity) {
	entity.child_of(_current_scene);
}

void WorldSim::set_active_scene(Entity scene) {
	fassert(_world.is_instance_of(scene.id(), _scene_prefab.id()), "Given scene isn't a scene instance");

	// Clear old active scene marker
	_world.remove_all(ActiveScene::get_class_static());

	scene.add<ActiveScene>();
	_current_scene = scene;
}

} //namespace feather
