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

	// Component types need no registration pass here: World subscribes to ClassDB for IComponent's children when it is
	// constructed, so everything reflected is already a component, and anything a project DLL registers later becomes
	// one as it arrives.
}

void WorldSim::init() {
	_scene_prefab = _world.prefab("Scene");
	auto scene = create_scene("new scene");
	fassert(scene.is_valid());
	set_active_scene(scene);

	// Only now, for two reasons: the world holds the scene content a module's systems will run against, and this runs
	// after index_project(), so a module from a loaded project DLL is picked up by the same sweep as core's.
	// The discovery itself belongs to World -- see World::import_modules.
	_world.import_modules();
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
	// Note this actually checks the prefab relationship now. The old spelling was `fassert(scene.is_a(_scene_prefab))`,
	// which set the relationship and tested the returned handle rather than asking anything.
	fassert(_world.is_instance_of(scene.id(), _scene_prefab.id()), "Given scene isn't a scene instance");

	// Clear old active scene marker
	_world.remove_all(ActiveScene::get_class_static());

	scene.add<ActiveScene>();
	_current_scene = scene;
}

} //namespace feather
