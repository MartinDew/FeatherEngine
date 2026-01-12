#include "engine.h"
#include "launch_settings.h"
#include "modules/modules.gen.h"

#include <framework/assert.h>

#include <chrono>

#include "rendering/rendering_server.h"
#include "resources/mesh.h"

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

struct SimulationTest {
	struct Entity {
		Transform transform;
		std::unique_ptr<Mesh> mesh;
		std::unique_ptr<Material> material;
	};

	std::vector<Entity> entities;
	Transform camera_transform;
	Projection camera_projection;

	SimulationTest() {
		entities.emplace_back(Transform { Vector3 { -5, 0, -10 }, Quaternion {}, Vector3::one },
				std::make_unique<BoxMesh>(), std::make_unique<PBRMaterial>());

		// Setup camera
		camera_transform = Transform { Vector3 { 0, 0, 0 }, Quaternion {}, Vector3::one };
		camera_projection = Projection::create_perspective(90.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
	}

	void update(double dt) {
		// Rotate each entity
		for (auto& entity : entities) {
			entity.transform.rotation = entity.transform.rotation *
					Quaternion::create_from_yaw_pitch_roll(Vector3 { 0, static_cast<real_t>(dt), 0 });
		}
	}

	RenderCapture generate_render_capture() const {
		static size_t frame = 0;
		RenderCapture capture { frame++ };
		// Set camera
		capture.set_camera_transform(camera_transform);
		capture.set_camera_projection(camera_projection);

		// Reserve space for entities
		capture.reserve_entities(entities.size());

		// Add all entities to the render capture
		for (const auto& entity : entities) {
			capture.add_entity(RenderCapture::EntityRender { .transform = entity.transform,
					.triangle_mesh = *entity.mesh,
					.material = *entity.material,
					.entity_id = 0, // You could add an ID field to Entity if needed
					.cast_shadows = true,
					.receive_shadows = true });
		}

		auto dir = Vector3 { 0.5f, -1.0f, 0.5f };
		dir.normalize();

		// Add a basic directional light
		capture.add_light(RenderCapture::Light { .type = RenderCapture::Light::Type::Directional,
				.position = Vector3::zero,
				.direction = dir,
				.color = Color(1.0f, 1.0f, 1.0f, 1.0f),
				.intensity = 1.0f,
				.cast_shadows = true });

		return capture;
	}
};

bool Engine::run() {
	auto current_time = start_time;

	bool keep_running = true;

	// initialization

	// Debug stuff
	if (LaunchSettings::get().dump_db.Get()) {
		ClassDB::get().print_db();
		return true;
	}

	SimulationTest sim;

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
			// _physics_update here
			sim.update(_current_dt);
		}

		_current_dt = frame_time;
		// Update here

		// Tell the renderer to render here
		_rendering_server.set_render_capture(sim.generate_render_capture());
		_rendering_server.update(frame_time);
	}

	return true;
}

double Engine::get_current_delta_time() const { return _current_dt; }

bool Engine::is_editor() {
	if constexpr (!EDITOR_BUILD)
		return false;
	else
		return LaunchSettings::get().editor_mode.Get();
}
} //namespace feather