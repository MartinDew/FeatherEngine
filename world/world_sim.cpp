module;

#include <flecs.h>
#include <framework/singleton_helpers.h>

module feather.world;

import std;

namespace feather {

FSINGLETON_INSTANCE(WorldSim);

WorldSim::WorldSim() : fixed_tick { _world.timer().interval(Engine::simulation_time) } {
	FSINGLETON_CONSTRUCT_INSTANCE()
#if BETA
	_world.set<flecs::Rest>({});
#endif
}

WorldSim::~WorldSim() {
	ClassDB::unregister_subclass_delegate(EcsModule::get_class_static(), _subclass_delegate_id);
	ClassDB::unregister_class_delegate(_class_delegate_id);
}

void WorldSim::init() {
	// Components are matched to typed flecs use by the compiler's name for the type, which ClassInfo derives the way
	// flecs does. A compiler or flecs update that changes either spelling fails here rather than duplicating components.
	fassert(
			ClassDB::get_class_info<Scene>()->cpp_name.str() == std::string_view(flecs::_::type_name<Scene>()),
			std::format(
					"ClassInfo names Scene '{}' but flecs names it '{}'",
					ClassDB::get_class_info<Scene>()->cpp_name.str(),
					flecs::_::type_name<Scene>()
			)
	);

	// Every value type is a component, registered before any EcsModule imports and uses them by type.
	_world.component<ReflectedComponent>("ReflectedComponent");
	_world.register_reflected_components();
	_class_delegate_id = ClassDB::on_class_registered([this](std::string_view class_name) {
		_world.register_component(StaticString(class_name));
	});

	// Not "Scene": that name already resolves to the Scene component's entity.
	_scene_prefab = _world.prefab("ScenePrefab");
	auto scene = create_scene("new scene");
	fassert(scene.is_valid());
	set_active_scene(scene);

	// Subscribed here, not in the ctor (which runs before index_project()):
	// registering a class fires this immediately, so a DLL's EcsFeature would otherwise import reentrantly, before the
	// world above exists.
	_subclass_delegate_id = ClassDB::on_subclass_registered(
			EcsModule::get_class_static(), [world_sim = this](std::string_view class_name) {
				if (world_sim) {
					ClassDB::get_static_method(class_name, "_import_module").call(world_sim);
				}
			}
	);

	// Picks up EcsModule subclasses from core, from built-in modules, and --
	// because this runs after index_project() -- from loaded project DLLs.
	auto children = ClassDB::get_children_names(EcsModule::get_class_static());
	for (auto& child : children) {
		ClassDB::get_static_method(child, "_import_module").call(this);
	}
}

void WorldSim::update(double delta) {
	bool result = _world.progress(/*delta*/);
}

Entity WorldSim::create_scene(const std::string& name) const {
	Scene s { { name } };
	return _world.entity(name.c_str()).is_a(_scene_prefab).set<Scene>(s);
}

void WorldSim::add_to_scene(Entity entity) const {
	entity.child_of(_current_scene);
}

bool WorldSim::_is_in_scene(flecs::entity e, Entity scene) const {
	flecs::entity current = e;
	while (current.is_valid()) {
		if (current == scene)
			return true;
		current = current.parent();
	}
	return false;
}

void WorldSim::set_active_scene(Entity scene) {
	fassert(scene.is_a(_scene_prefab), "Given scene isn't a scene instance");

	// Clear old active scene marker
	_world.remove_all<ActiveScene>();

	scene.add<ActiveScene>();
	_current_scene = scene;
}

} //namespace feather
