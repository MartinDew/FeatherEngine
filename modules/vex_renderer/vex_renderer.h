#pragma once

#include <core/framework/reflection_macros.h>
#include <core/math/math_defs.h>
#include <core/rendering/render_capture.h>
#include <core/rendering/renderer.h>
#include <array>
#include <unordered_map>
#include <vector>

#include <Vex.h>

namespace feather {

class Texture;
class TriangleMesh;

class VexRenderer : public Renderer {
	FCLASS(VexRenderer, Renderer);

	// Core rendering resources
	vex::Texture depthTexture;
	vex::Graphics graphics;

	// Shadow maps
	std::vector<vex::Texture> _shadow_maps;
	std::unordered_map<uint32_t, size_t> _light_to_shadow_map_index;

	// GPU buffers
	vex::Buffer _camera_uniform_buffer;
	vex::Buffer _lights_structured_buffer;
	vex::Buffer _per_entity_uniform_buffer;

	// Resource caches
	struct MeshBuffers {
		vex::Buffer vertex_buffer;
		vex::Buffer index_buffer;
		uint32_t index_count = 0;
	};
	struct TextureGPUData {
		vex::Texture texture;
		vex::BindlessHandle bindless_handle;
	};
	std::unordered_map<const TriangleMesh*, MeshBuffers> _mesh_cache;
	std::unordered_map<const Texture*, TextureGPUData> _texture_cache;

	// Default textures
	vex::Texture _default_white_texture;
	vex::Texture _default_normal_texture;
	vex::Texture _default_metallic_roughness_texture;
	vex::BindlessHandle _default_white_handle;
	vex::BindlessHandle _default_normal_handle;
	vex::BindlessHandle _default_mr_handle;

	// Shader pipeline states
	vex::DrawDesc _pbr_draw_desc;
	vex::DrawDesc _shadow_draw_desc;

	// Helper methods
	void _render_shadow_pass(const RenderCapture& capture, vex::CommandContext& ctx);
	void _render_forward_pass(const RenderCapture& capture, vex::CommandContext& ctx);
	void _upload_camera_uniforms(const RenderCapture& capture, vex::CommandContext& ctx);
	void _upload_lights_buffer(const RenderCapture& capture, vex::CommandContext& ctx);
	MeshBuffers& _get_or_create_mesh_buffers(const TriangleMesh& mesh, vex::CommandContext& ctx);
	TextureGPUData& _get_or_create_texture(const Texture* texture, vex::CommandContext& ctx);
	vex::BindlessHandle _get_texture_handle(
			const Texture* texture, vex::CommandContext& ctx, vex::BindlessHandle default_handle);
	Matrix _compute_light_view_proj(const RenderCapture::Light& light, const RenderCapture& capture);

	// Utility methods
	Vector3 _compute_scene_center(const RenderCapture& capture);
	float _compute_scene_radius(const RenderCapture& capture, const Vector3& center);
	Matrix _compute_normal_matrix(const Matrix& modelMatrix);

	static vex::PlatformWindowHandle _create_vex_window(Window& window);

protected:
	void _render_scene(RenderCapture capture) override;
	void _on_resize() override;

	static void _bind_members();

public:
	VexRenderer();
};

} //namespace feather
