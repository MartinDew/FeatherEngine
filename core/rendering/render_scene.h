#pragma once;

#include "framework/cow_vector.h"
#include "math/transform.h"
#include "mesh_data.h"
#include "rendering/viewport.h"
#include "resources/material.h"

namespace feather {

class MeshData;
class Material;

struct Light;

class RenderScene : public Reflected {
	FCLASS(RenderScene, Reflected)
public:
	struct EntityRender {
		Transform transform;
		const std::shared_ptr<MeshData> triangle_mesh;
		const std::shared_ptr<Material> material;
		uint32_t entity_id = 0; // For debugging/identification
		bool cast_shadows = true;
		bool receive_shadows = true;
	};

	RenderScene();

	// With COW vectors, copies are cheap - they just increment ref count
	RenderScene(const RenderScene&);
	RenderScene& operator=(const RenderScene&);
	RenderScene(RenderScene&&) noexcept;
	RenderScene& operator=(RenderScene&&) noexcept;

	// Viewport (camera + screen rect)
	const Viewport& get_viewport() const noexcept;
	void set_viewport(const Viewport& viewport);

	// Entity management
	void add_entity(const EntityRender& entity);
	void reserve_entities(size_t count);

	const CowVector<EntityRender>& get_entities() const noexcept;
	size_t get_entity_count() const noexcept;

	// Light management
	void add_light(const Light& light);
	void reserve_lights(size_t count);

	const CowVector<Light>& get_lights() const noexcept;
	size_t get_light_count() const noexcept;

	// Clear for reuse (triggers copy-on-write if shared)
	void clear();

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

	const EnvironmentSettings& get_environment() const noexcept;
	void set_environment(const EnvironmentSettings& env);

protected:
	static void _bind_members();

private:
	Viewport _viewport;
	CowVector<EntityRender> _entities;
	CowVector<Light> _lights;
	EnvironmentSettings _environment;
};

} //namespace feather
