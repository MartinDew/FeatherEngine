#pragma once;

#include "framework/cow_vector.h"
#include "math/projection.h"
#include "math/transform.h"
#include "resources/material.h"
#include "triangle_mesh.h"

namespace feather {

class TriangleMesh;
class Material;

class RenderScene {
public:
	struct EntityRender {
		Transform transform;
		const std::shared_ptr<TriangleMesh> triangle_mesh;
		const std::shared_ptr<Material> material;
		uint32_t entity_id = 0; // For debugging/identification
		bool cast_shadows = true;
		bool receive_shadows = true;
	};

	// Todo will probably move
	struct Light {
		enum class Type : uint8_t {
			Directional,
			Point,
			Spot
		};

		Type type = Type::Directional;
		Vector3 position = Vector3::zero;
		Vector3 direction = Vector3::forward;
		Color color = Color(1.0f, 1.0f, 1.0f, 1.0f);
		float intensity = 1.0f;
		float range = 100.0f; // For point/spot lights
		float spot_angle = 45.0f; // For spot lights (in degrees)
		bool cast_shadows = true;
	};

	RenderScene(size_t frame_index = 0);

	// With COW vectors, copies are cheap - they just increment ref count
	RenderScene(const RenderScene&) = default;
	RenderScene& operator=(const RenderScene&) = default;
	RenderScene(RenderScene&&) noexcept = default;
	RenderScene& operator=(RenderScene&&) noexcept = default;

	// Camera
	const Transform& get_camera_transform() const noexcept { return _camera_transform; }
	void set_camera_transform(const Transform& transform) { _camera_transform = transform; }
	const Projection& get_camera_projection() const noexcept { return _camera_projection; }
	void set_camera_projection(const Projection& projection) { _camera_projection = projection; }

	// Entity management
	void add_entity(const EntityRender& entity) { _entities.push_back(entity); }
	void reserve_entities(size_t count) { _entities.reserve(count); }

	const CowVector<EntityRender>& get_entities() const noexcept { return _entities; }
	size_t get_entity_count() const noexcept { return _entities.size(); }

	// Light management
	void add_light(const Light& light) { _lights.push_back(light); }
	void reserve_lights(size_t count) { _lights.reserve(count); }

	const CowVector<Light>& get_lights() const noexcept { return _lights; }
	size_t get_light_count() const noexcept { return _lights.size(); }

	// Clear for reuse (triggers copy-on-write if shared)
	void clear() {
		_entities.clear();
		_lights.clear();
		_frame_index++;
	}

	// Frame tracking
	uint64_t get_frame_index() const noexcept { return _frame_index; }

	// Todo will probably move
	// Environment/Scene settings
	struct EnvironmentSettings {
		Color ambient_color = Color(0.1f, 0.1f, 0.1f, 1.0f);
		Color fog_color = Color(0.5f, 0.6f, 0.7f, 1.0f);
		float fog_density = 0.0f;
		float fog_start = 0.0f;
		float fog_end = 1000.0f;
		// Could add skybox, IBL probes, etc.
	};

	const EnvironmentSettings& get_environment() const noexcept { return _environment; }
	void set_environment(const EnvironmentSettings& env) { _environment = env; }

private:
	Transform _camera_transform;
	Projection _camera_projection;
	CowVector<EntityRender> _entities;
	CowVector<Light> _lights;
	EnvironmentSettings _environment;
	size_t _frame_index = 0;
};

} //namespace feather
