module;

#include <flecs.h>
#include <csignal>

module feather.main;

import std.compat;

namespace feather {

namespace {

// Headless has no window which in turn can't deliver quit events.
// this is the only exit condition. sig_atomic_t is the only type a signal
// handler may portably touch.
volatile std::sig_atomic_t quit_requested = 0;

void _on_terminate_signal(int) {
	quit_requested = 1;
}

using Clock = std::chrono::high_resolution_clock;

#if BETA
void _setup_demo_scene(WorldSim& _world_sim) {
	// test script
	auto w = *_world_sim.get_world();
	Transform t1 { { 0, -1, -3 }, Quaternion::from_euler(1.f, 0, 0), Vector3::one };
	Transform t2 { { -2, -1, -3 }, Quaternion::from_euler(1.f, 0, 0), Vector3::one };
	Transform t3 { { 2, -1, -3 }, Quaternion::from_euler(1.f, 0, 0), Vector3::one };
	Transform t4 { { 0, 2, -3 }, Quaternion::from_euler(1.f, 0, 0), Vector3::one };

	auto material = std::make_shared<PBRMaterial>();
	material->set_base_color_factor({ .7f, .7f, .0f });

	struct Move {};
	auto s = _world_sim.create_scene("Ni");
	_world_sim.set_active_scene(s);

	auto _ = _world_sim.create_entity(s, "Box1")
					 .emplace<Transform>(t1)
					 .emplace<MeshInstance>(std::make_shared<BoxMesh>())
					 .emplace<MaterialInstance>(material)
					 .add<Move>();

	_ = _world_sim.create_entity(s, "Box2")
				.emplace<Transform>(t2)
				.emplace<MeshInstance>(std::make_shared<BoxMesh>())
				.emplace<MaterialInstance>(material)
				.add<Move>();

	w.entity(s, "Box3")
			.emplace<Transform>(t3)
			.emplace<MeshInstance>(std::make_shared<BoxMesh>())
			.emplace<MaterialInstance>(material)
			.add<Move>();

	w.entity("BoxChild").emplace<Transform>(t4).emplace<MeshInstance>(std::make_shared<BoxMesh>()).child_of(_);

	_world_sim.create_entity(s, "Floor")
			.emplace<Transform>(Vector3 { 0, -2, 0 }, Quaternion::identity, Vector3 { 200, 0.1f, 200 })
			.emplace<MeshInstance>(std::make_shared<BoxMesh>());

	auto dir = Vector3 { -0.5f, -1.0f, -1.f };
	Light l { .type = LightType::Directional,
			  .position = Vector3::zero,
			  .direction = dir,
			  .color = Color(1.0f, 1.0f, 1.0f, 1.0f),
			  .intensity = 10.0f,
			  .cast_shadows = true };
	w.entity(s, "Directional").emplace<Light>(std::move(l));

	w.system<const MeshInstance, Transform>("Spin")
			.with<Move>()
			.kind(flecs::OnUpdate)
			.write<Transform>()
			.each([](flecs::iter& it, size_t, const MeshInstance& mi, Transform& t) {
				t.rotation = t.rotation * Quaternion::from_euler(static_cast<real_t>(it.delta_time()), 0, 0);
			});

	auto q = w.query_builder<Transform, MeshInstance, MaterialInstance*>("Test")
					 .with<ActiveScene>()
					 .optional()
					 .parent()
					 .cascade()
					 .build();

	q.each([](Entity e, Transform& t, MeshInstance mi, MaterialInstance* mat) { std::cout << e.name() << std::endl; });
}
#endif

bool _run(Engine& engine, WorldSim& world_sim, RenderingServer& rendering_server) {
	auto current_time = Clock::now();

	// initialization
	ResourceLoader::get()->index_project();

#if BETA
	if (LaunchSettings::get().dump_db.Get()) {
		ClassDB::get()->print_db();
		return true;
	}
#endif

	world_sim.init();

#if BETA
	if (LaunchSettings::get().demo_mode.Get()) {
		_setup_demo_scene(world_sim);
	}
#endif

	// Installed in every mode: Ctrl+C on a windowed session should also shut
	// down through normal teardown, not just headless.
	std::signal(SIGINT, &_on_terminate_signal);
	std::signal(SIGTERM, &_on_terminate_signal);

	double accumulator = 0.0;
	bool keep_running = true;
	while (keep_running && !quit_requested) {
		keep_running = engine.get_main_window().update();

		auto new_time = Clock::now();

		double frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - current_time).count();
		current_time = new_time;

		accumulator += frame_time;
		engine.set_current_delta_time(Engine::simulation_time);
		while (accumulator >= Engine::simulation_time) {
			accumulator -= Engine::simulation_time;
			world_sim.fixed_update(Engine::simulation_time);
		}

		engine.set_current_delta_time(frame_time);
		world_sim.update(frame_time);

		rendering_server.update(frame_time);
	}

	rendering_server.stop();

	return true;
}

} //namespace

struct Main {
	ClassDB _class_db;
	LaunchSettings _launch_settings;
	ProjectSettings _project_settings;
	ResourceLoader _resource_loader;

	ModuleHooks _hooks;

	Main(int argc, char* argv[], ModuleHooks hooks);
	~Main();
	void setup_db();
};

Main::Main(int argc, char* argv[], ModuleHooks hooks)
		: _class_db()
		, _launch_settings(std::move(argc), std::move(argv))
		, _hooks(hooks) {
	if (!_project_settings.init() && !_launch_settings.demo_mode.Get())
		return;

	setup_db();

	Engine engine;
	RenderingServer rendering_server;
	WorldSim world_sim;
	rendering_server.init();

	_run(engine, world_sim, rendering_server);

	_hooks.unregister_modules();
}

Main::~Main() {
	ClassDB::clear();
}

void Main::setup_db() {
	register_framework_types();
	register_math_types();
	register_reflection_types();
	register_core_types();
	register_resources_types();
	register_rendering_types();
	register_world_types();
	register_main_types();

	// then register module types
	_hooks.register_modules();
}

int run(int argc, char* argv[], ModuleHooks hooks) {
	Main fmain(argc, argv, hooks);
	return 0;
}

} //namespace feather
