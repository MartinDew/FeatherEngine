#pragma once

#include <world/ecs_defs.h>
#include <world/components/scene.h>
#include <flecs.h>

#include <string>
#include <vector>

namespace feather {

class World {
	flecs::world _flecs;
	EcsTimer     _fixed_tick;

	Entity _scene_prefab;
	Entity _current_scene;
	std::vector<Entity> _scenes;

public:
	World() = default;
	World(flecs::world w) : _flecs(w) {}

	// --- Flecs registration passthrough ---

	template<class T>
	auto component(const char* name = nullptr) {
		return name ? _flecs.component<T>(name) : _flecs.component<T>();
	}

	template<class... TComps>
	auto system(const char* name = nullptr) {
		return name ? _flecs.system<TComps...>(name) : _flecs.system<TComps...>();
	}

	template<class... TComps>
	auto query_builder(const char* name = nullptr) {
		return name ? _flecs.query_builder<TComps...>(name) : _flecs.query_builder<TComps...>();
	}

	template<class T>
	auto module(const char* name = nullptr) {
		return name ? _flecs.module<T>(name) : _flecs.module<T>();
	}

	template<class T>
	auto import_module() { return _flecs.import<T>(); }

	template<class T>
	void set(T val) { _flecs.set(val); }

	template<class T>
	void remove_all() { _flecs.remove_all<T>(); }

	auto prefab(const char* name) { return _flecs.prefab(name); }
	auto timer() { return _flecs.timer(); }

	Entity entity(const char* name = nullptr);
	Entity entity(Entity scene, const char* name);

	void progress(double delta);

	// --- Scene management ---

	void init_scenes();
	[[nodiscard]] Entity create_scene(std::string name);
	[[nodiscard]] Entity create_entity(std::string name = "") const;
	[[nodiscard]] Entity create_entity(const Entity& parent, std::string name = "") const;
	void add_to_scene(Entity entity) const;
	void set_active_scene(Entity scene);
	[[nodiscard]] Entity& get_current_scene() { return _current_scene; }

	// --- Scene-scoped query ---

	template<class... TComps>
	auto scene_query() {
		return _flecs.query_builder<TComps...>()
			.template with<ActiveScene>().up(Ecs::ChildOf)
			.build();
	}

	// --- Fixed tick ---

	void set_fixed_tick_interval(double interval);

	template<class... TComps>
	auto& execute_fixed(Ecs::system_builder<TComps...>& system) {
		return system.tick_source(_fixed_tick);
	}

	// --- Low-level access ---

	[[nodiscard]] flecs::world& flecs_world() { return _flecs; }
};

} // namespace feather