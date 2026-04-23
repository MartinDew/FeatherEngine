#include "engine.h"
#include "launch_settings.h"
#include "world/components/light.h"
#include "world/rendering_world_feature.h"

#include <framework/assert.h>

#include <chrono>

#include <rendering/rendering_server.h>
#include <resources/mesh.h>

namespace feather {

namespace {

constexpr double simulation_time = 1.0 / 60.0;

} //namespace

Engine* Engine::_instance = nullptr;

Engine::Engine() {
	// Todo replace sdl assert by custom one
	fassert(!_instance);

	_instance = this;
	_rendering_server.init();
}

Engine::~Engine() {
	// unregister_modules();
}

bool Engine::run() {
	auto current_time = start_time;

	bool keep_running = true;

	// initialization

	// Debug stuff
	if (LaunchSettings::get().dump_db.Get()) {
		ClassDB::get()->print_db();
		return true;
	}

	_world_sim.init();

	// test script
	{
		auto w = *_world_sim.get_world();
		Transform t1 { { 0, -1, -3 }, Quaternion::create_from_yaw_pitch_roll(1.f, 0, 0), Vector3::one };
		Transform t2 { { -2, -1, -3 }, Quaternion::create_from_yaw_pitch_roll(1.f, 0, 0), Vector3::one };
		Transform t3 { { 2, -1, -3 }, Quaternion::create_from_yaw_pitch_roll(1.f, 0, 0), Vector3::one };

		auto material = std::make_shared<PBRMaterial>();
		material->set_base_color_factor({ .7f, .7f, .0f });

		struct Move {};
		w.entity("Box1")
				.emplace<Transform>(t1)
				.emplace<MeshInstance>(std::make_shared<BoxMesh>())
				.emplace<MaterialInstance>(material)
				.add<Move>();
		w.entity("Box2")
				.emplace<Transform>(t2)
				.emplace<MeshInstance>(std::make_shared<BoxMesh>())
				.emplace<MaterialInstance>(material)
				.add<Move>();
		w.entity("Box3")
				.emplace<Transform>(t3)
				.emplace<MeshInstance>(std::make_shared<BoxMesh>())
				.emplace<MaterialInstance>(material)
				.add<Move>();

		/*
		* Transform { Vector3 { 0, -2, 0 },
							Quaternion::create_from_yaw_pitch_roll({ 0, 0, 0 }),
							Vector3 { 200, 0.1f, 200 } },
				std::make_shared<BoxMesh>(),
		 */
		w.entity("Floor")
				.emplace<Transform>(
						Vector3 { 0, -2, 0 },
						Quaternion::create_from_yaw_pitch_roll({ 0, 0, 0 }),
						Vector3 { 200, 0.1f, 200 }
				)
				.emplace<MeshInstance>(std::make_shared<BoxMesh>());

		auto dir = Vector3 { -0.5f, -1.0f, -1.f };
		Light l { .type = Light::Type::Directional,
				  .position = Vector3::zero,
				  .direction = dir,
				  .color = Color(1.0f, 1.0f, 1.0f, 1.0f),
				  .intensity = 10.0f,
				  .cast_shadows = true };
		w.entity("Directional").emplace<Light>(std::move(l));

		w.system<const MeshInstance, Transform>("Spin")
				.with<Move>()
				.kind(flecs::OnUpdate)
				.write<Transform>()
				.each([](flecs::iter& it, size_t, const MeshInstance& mi, Transform& t) {
					t.rotation = t.rotation *
							Quaternion::create_from_yaw_pitch_roll(
										 Vector3 { 0, static_cast<real_t>(it.delta_time()), 0 }
							);
				});
	}

	// update
	double accumulator = 0.0;
	while (keep_running) {
		keep_running = _main_window.update();

		auto new_time = Clock::now();

		double frame_time = std::chrono::duration_cast<std::chrono::duration<double>>(new_time - current_time).count();
		current_time = new_time;

		accumulator += frame_time;
		_current_dt = simulation_time;
		while (accumulator >= simulation_time) {
			accumulator -= simulation_time;
			_world_sim.fixed_update(simulation_time);
		}

		_current_dt = frame_time;
		_world_sim.update(frame_time);

		// Tell the renderer to render here
		// _rendering_server.set_render_capture(sim.generate_render_capture());
		_rendering_server.update(frame_time);
	}

	return true;
}

double Engine::get_current_delta_time() const {
	return _current_dt;
}

bool Engine::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}
} //namespace feather