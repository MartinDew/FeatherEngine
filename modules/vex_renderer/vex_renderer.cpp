#include "vex_renderer.h"

#include "Vex/Shaders/ShaderKey.h"
#include "Vex/Texture.h"
#include "framework/bytes.h"
#include <core/main/engine.h>
#include <core/main/engine_settings.h>
#include <core/main/window.h>
#include <core/math/math_defs.h>
#include <core/resources/material.h>
#include <core/resources/texture.h>

#include <raw_resources/shaders/pbr_forward.slang.gen.h>
#include <raw_resources/shaders/shadow_depth.slang.gen.h>

#include <array>
#include <cstdint>
#include <span>

namespace feather {

using namespace vex;

vex::PlatformWindowHandle VexRenderer::_create_vex_window(Window& window) {
	Window& engine_window = Engine::get().get_main_window();
	SDL_Window* internal_window = Renderer::_extract_internal_window(engine_window);

	PlatformWindowHandle vex_window;
	auto pid = SDL_GetWindowProperties(internal_window);
#if (__linux__)
	if (SDL_HasProperty(pid, SDL_PROP_WINDOW_X11_WINDOW_NUMBER)) {
		vex_window = { PlatformWindowHandle::X11Handle {
				.window = static_cast<::Window>(SDL_GetNumberProperty(pid, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, {})),
				.display = static_cast<Display*>(
						SDL_GetPointerProperty(pid, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr)),
		} };
	}
	else if (SDL_HasProperty(pid, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER)) {
		vex_window = { PlatformWindowHandle::WaylandHandle {
				.window = static_cast<::wl_surface*>(
						SDL_GetPointerProperty(pid, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr)),
				.display = static_cast<::wl_display*>(
						SDL_GetPointerProperty(pid, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr)) } };
	}
	else {
		fassert(false, "VexRenderer : No supported video driver found");
	}
#elif (_WIN32)
	using NativeWindow = HWND;
	vex_window = { PlatformWindowHandle::WindowsHandle { .window = static_cast<NativeWindow>(SDL_GetPointerProperty(
																 pid, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr)) } };

#elifdef __APPLE__
	// macOS implementation would go here
#endif

	return vex_window;
}

struct CameraUniforms {
	Matrix viewProj;
	Vector3 cameraPos;
	float _padding;
};

struct EntityUniforms {
	Matrix model;
	Matrix normalMatrix;
	Color baseColorFactor;
	float metallicFactor;
	float roughnessFactor;
	float _padding1;
	Color emissiveFactor;
	vex::BindlessHandle baseColorHandle;
	vex::BindlessHandle metallicRoughnessHandle;
	vex::BindlessHandle normalHandle;
	vex::BindlessHandle emissiveHandle;
};

void VexRenderer::_bind_members() {}

static const std::filesystem::path shader_path = std::filesystem::current_path() / "shaders";

VexRenderer::VexRenderer()
		: graphics(vex::GraphicsCreateDesc {
				  .platformWindow = { .windowHandle = _create_vex_window(Engine::get().get_main_window()),
						  .width = static_cast<uint32_t>(Engine::get().get_main_window().properties.width),
						  .height = static_cast<uint32_t>(Engine::get().get_main_window().properties.height) },
				  .enableGPUDebugLayer = !VEX_SHIPPING,
				  .enableGPUBasedValidation = !VEX_SHIPPING }) {
	auto main_window = Engine::get().get_main_window();
	uint32_t width = main_window.properties.width;
	uint32_t height = main_window.properties.height;

	// Depth texture
	depthTexture = graphics.CreateTexture({
			.name = "Depth Texture",
			.type = vex::TextureType::Texture2D,
			.format = vex::TextureFormat::D32_FLOAT,
			.width = width,
			.height = height,
			.usage = vex::TextureUsage::DepthStencil,
			.clearValue =
					vex::TextureClearValue {
							.clearAspect = vex::TextureAspect::Depth,
							.depth = 0,
					},
	});

	vex::CommandContext ctx = graphics.CreateCommandContext(vex::QueueType::Graphics);

	// Create GPU buffers
	_camera_uniform_buffer =
			graphics.CreateBuffer(vex::BufferDesc::CreateUniformBufferDesc("Camera Uniforms", sizeof(CameraUniforms)));
	_lights_structured_buffer = graphics.CreateBuffer(vex::BufferDesc::CreateStructuredBufferDesc("Lights Buffer", 80));
	_per_entity_uniform_buffer = graphics.CreateBuffer(
			vex::BufferDesc::CreateUniformBufferDesc("Per-Entity Uniforms", sizeof(EntityUniforms)));

	// Create default textures
	// White 1x1 texture
	{
		std::array<uint8_t, 4> whitePixel = { 255, 255, 255, 255 };
		_default_white_texture = graphics.CreateTexture({ .name = "Default White",
				.type = vex::TextureType::Texture2D,
				.format = vex::TextureFormat::RGBA8_UNORM,
				.width = 1,
				.height = 1,
				.usage = vex::TextureUsage::ShaderRead });
		std::span bytes = to_bytes(whitePixel);
		ctx.EnqueueDataUpload(_default_white_texture, bytes, vex::TextureRegion::SingleMip(0));
		ctx.Barrier(_default_white_texture, vex::RHIBarrierSync::PixelShader, vex::RHIBarrierAccess::ShaderRead,
				vex::RHITextureLayout::ShaderResource);
		_default_white_handle = graphics.GetBindlessHandle(vex::TextureBinding {
				.texture = _default_white_texture, .usage = vex::TextureBindingUsage::ShaderRead });
	}

	// Default normal map (0.5, 0.5, 1.0, 1.0)
	{
		std::array<uint8_t, 4> normalPixel = { 128, 128, 255, 255 };
		_default_normal_texture = graphics.CreateTexture({ .name = "Default Normal",
				.type = vex::TextureType::Texture2D,
				.format = vex::TextureFormat::RGBA8_UNORM,
				.width = 1,
				.height = 1,
				.usage = vex::TextureUsage::ShaderRead });
		ctx.EnqueueDataUpload(_default_normal_texture, to_bytes(normalPixel), vex::TextureRegion::SingleMip(0));
		ctx.Barrier(_default_normal_texture, vex::RHIBarrierSync::PixelShader, vex::RHIBarrierAccess::ShaderRead,
				vex::RHITextureLayout::ShaderResource);
		_default_normal_handle = graphics.GetBindlessHandle(vex::TextureBinding {
				.texture = _default_normal_texture, .usage = vex::TextureBindingUsage::ShaderRead });
	}

	// Default metallic/roughness (0, 1, 0, 0) - non-metallic, rough
	{
		std::array<uint8_t, 4> mrPixel = { 0, 255, 0, 0 };
		_default_metallic_roughness_texture = graphics.CreateTexture({ .name = "Default Metallic/Roughness",
				.type = vex::TextureType::Texture2D,
				.format = vex::TextureFormat::RGBA8_UNORM,
				.width = 1,
				.height = 1,
				.usage = vex::TextureUsage::ShaderRead });
		ctx.EnqueueDataUpload(_default_metallic_roughness_texture, to_bytes(mrPixel), vex::TextureRegion::SingleMip(0));
		ctx.Barrier(_default_metallic_roughness_texture, vex::RHIBarrierSync::PixelShader,
				vex::RHIBarrierAccess::ShaderRead, vex::RHITextureLayout::ShaderResource);
		_default_mr_handle = graphics.GetBindlessHandle(vex::TextureBinding {
				.texture = _default_metallic_roughness_texture, .usage = vex::TextureBindingUsage::ShaderRead });
	}

	// Setup samplers
	std::array samplers {
		vex::TextureSampler::CreateSampler(vex::FilterMode::Linear, vex::AddressMode::Clamp),
		vex::TextureSampler::CreateSampler(vex::FilterMode::Point, vex::AddressMode::Clamp),
	};
	graphics.SetSamplers(samplers);

	// Load and compile shaders
	vex::VertexInputLayout pbrVertexLayout {
		.attributes = {
			{
				.semanticName = "POSITION",
				.semanticIndex = 0,
				.binding = 0,
				.format = vex::TextureFormat::RGB32_FLOAT,
				.offset = 0,
			},
			{
				.semanticName = "NORMAL",
				.semanticIndex = 0,
				.binding = 0,
				.format = vex::TextureFormat::RGB32_FLOAT,
				.offset = sizeof(float) * 3,
			}
		},
		.bindings = {
			{
				.binding = 0,
				.strideByteSize = static_cast<uint32_t>(sizeof(Vertex)),
				.inputRate = vex::VertexInputLayout::InputRate::PerVertex,
			},
		},
	};

	vex::DepthStencilState depthStencilState {
		.depthTestEnabled = true,
		.depthWriteEnabled = true,
		.depthCompareOp = vex::CompareOp::GreaterEqual, // Reverse-Z
	};

	// PBR forward rendering pipeline
	_pbr_draw_desc = vex::DrawDesc {
		.vertexShader = {
			.path = shader_path / "pbr_forward.slang",
			.entryPoint = "VSMain",
			.type = vex::ShaderType::VertexShader,
			.compiler = ShaderCompilerBackend::Slang,
		},
		.pixelShader = {
			.path = shader_path / "pbr_forward.slang",
			.entryPoint = "PSMain",
			.type = vex::ShaderType::PixelShader,
			.compiler = ShaderCompilerBackend::Slang,
		},
		.vertexInputLayout = pbrVertexLayout,
		.depthStencilState = depthStencilState,
	};

	// Shadow depth pipeline (vertex-only, no pixel shader for depth-only)
	_shadow_draw_desc = vex::DrawDesc {
		.vertexShader = {
			.path = shader_path / "shadow_depth.slang",
			.entryPoint = "VSMain",
			.type = vex::ShaderType::VertexShader,
			.compiler = ShaderCompilerBackend::Slang,
		},
		.pixelShader = {
			.path = shader_path / "shadow_depth.slang",
			.entryPoint = "PSMain",
			.type = vex::ShaderType::PixelShader,
			.compiler = ShaderCompilerBackend::Slang,
		},
		.vertexInputLayout = pbrVertexLayout,
		.depthStencilState = depthStencilState,
	};

	// Pre-allocate shadow maps (can expand dynamically)
	for (int i = 0; i < 4; ++i) {
		_shadow_maps.push_back(graphics.CreateTexture({
				.name = "Shadow Map",
				.type = vex::TextureType::Texture2D,
				.format = vex::TextureFormat::D32_FLOAT,
				.width = 1024,
				.height = 1024,
				.usage = vex::TextureUsage::DepthStencil | vex::TextureUsage::ShaderRead,
				.clearValue =
						vex::TextureClearValue {
								.clearAspect = vex::TextureAspect::Depth,
								.depth = 0,
						},
		}));
	}

	graphics.Submit(ctx);
}

void VexRenderer::_render_scene(const RenderCapture capture) {
	auto ctx = graphics.CreateCommandContext(vex::QueueType::Graphics);

	// Check if there are any shadow-casting lights
	bool hasShadows = false;
	for (const auto& light : capture.get_lights()) {
		if (light.cast_shadows) {
			hasShadows = true;
			break;
		}
	}

	// Shadow pass
	// if (hasShadows) {
	// 	VEX_GPU_SCOPED_EVENT(ctx, "Shadow Pass");
	// 	_render_shadow_pass(capture, ctx);
	// }

	// Forward pass
	{
		VEX_GPU_SCOPED_EVENT(ctx, "Forward Pass");
		_render_forward_pass(capture, ctx);
	}

	graphics.Submit(ctx);
	graphics.Present(_window->fullscreen_mode == Window::FullscreenMode::FULLSCREEN);
}

void VexRenderer::_on_resize() {
	auto width = _window->properties.width;
	auto height = _window->properties.height;

	if (width == 0 || height == 0) {
		return;
	}

	// Recreate depth texture
	depthTexture = graphics.CreateTexture({
			.name = "Depth Texture",
			.type = vex::TextureType::Texture2D,
			.format = vex::TextureFormat::D32_FLOAT,
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.usage = vex::TextureUsage::DepthStencil,
			.clearValue =
					vex::TextureClearValue {
							.clearAspect = vex::TextureAspect::Depth,
							.depth = 0,
					},
	});

	graphics.OnWindowResized(width, height);
}

// Shadow pass implementation
void VexRenderer::_render_shadow_pass(const RenderCapture& capture, vex::CommandContext& ctx) {
	const auto& lights = capture.get_lights();
	_light_to_shadow_map_index.clear();

	size_t shadow_map_index = 0;
	for (size_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		if (!light.cast_shadows)
			continue;

		// Ensure we have a shadow map
		if (shadow_map_index >= _shadow_maps.size()) {
			_shadow_maps.push_back(graphics.CreateTexture({
					.name = "Shadow Map",
					.type = vex::TextureType::Texture2D,
					.format = vex::TextureFormat::D32_FLOAT,
					.width = 1024,
					.height = 1024,
					.usage = vex::TextureUsage::DepthStencil | vex::TextureUsage::ShaderRead,
					.clearValue =
							vex::TextureClearValue {
									.clearAspect = vex::TextureAspect::Depth,
									.depth = 0,
							},
			}));
		}

		vex::Texture& shadowMap = _shadow_maps[shadow_map_index];
		_light_to_shadow_map_index[static_cast<uint32_t>(i)] = shadow_map_index++;

		// Compute light view-projection matrix
		Matrix lightVP = _compute_light_view_proj(light, capture);

		// Set render target (depth-only)
		ctx.ClearTexture(vex::TextureBinding { .texture = shadowMap });
		ctx.SetViewport(0, 0, 1024, 1024);
		ctx.SetScissor(0, 0, 1024, 1024);

		// Render entities
		const auto& entities = capture.get_entities();
		for (const auto& entity : entities) {
			if (!entity.cast_shadows)
				continue;

			// Get mesh buffers
			auto& meshBuffers = _get_or_create_mesh_buffers(entity.triangle_mesh, ctx);

			// Upload uniforms (MVP matrix from light's perspective)
			Matrix model = entity.transform.to_matrix_with_scale();
			Matrix mvp = model * lightVP;

			// Draw
			vex::BufferBinding vertexBufferBinding {
				.buffer = meshBuffers.vertex_buffer,
				.strideByteSize = static_cast<uint32_t>(sizeof(Vertex)),
			};
			vex::BufferBinding indexBufferBinding {
				.buffer = meshBuffers.index_buffer,
				.strideByteSize = static_cast<uint32_t>(sizeof(uint32_t)),
			};

			ctx.DrawIndexed(_shadow_draw_desc,
					{
							.depthStencil = vex::TextureBinding(shadowMap),
							.vertexBuffers = { &vertexBufferBinding, 1 },
							.indexBuffer = indexBufferBinding,
					},
					vex::ConstantBinding(mvp), meshBuffers.index_count);
		}

		// Transition shadow map to shader resource
		ctx.Barrier(shadowMap, vex::RHIBarrierSync::PixelShader, vex::RHIBarrierAccess::ShaderRead,
				vex::RHITextureLayout::ShaderResource);
	}
}

// Forward pass implementation
void VexRenderer::_render_forward_pass(const RenderCapture& capture, vex::CommandContext& ctx) {
	// Clear back buffer and depth
	auto backBuffer = graphics.GetCurrentPresentTexture();
	ctx.ClearTexture(vex::TextureBinding { .texture = backBuffer },
			vex::TextureClearValue { .clearAspect = vex::TextureAspect::Color, .color = { 0.2f, 0.2f, 0.2f, 1.0f } });
	ctx.ClearTexture(vex::TextureBinding { .texture = depthTexture });

	ctx.SetViewport(0, 0, _window->properties.width, _window->properties.height);
	ctx.SetScissor(0, 0, _window->properties.width, _window->properties.height);

	// Upload camera uniforms
	_upload_camera_uniforms(capture, ctx);

	// Upload lights buffer
	_upload_lights_buffer(capture, ctx);

	// Render entities
	const auto& entities = capture.get_entities();
	for (const auto& entity : entities) {
		// Get mesh buffers
		auto& meshBuffers = _get_or_create_mesh_buffers(entity.triangle_mesh, ctx);

		if (!&entity.material)
			continue;

		// Get material (try to cast to PBRMaterial)
		const PBRMaterial* pbrMat = object_cast<const PBRMaterial>(entity.material.get());

		// Get texture handles
		vex::BindlessHandle baseColorHandle = _get_texture_handle(
				pbrMat ? pbrMat->get_base_color_texture().get() : nullptr, ctx, _default_white_handle);
		vex::BindlessHandle metallicRoughnessHandle = _get_texture_handle(
				pbrMat ? pbrMat->get_metallic_roughness_texture().get() : nullptr, ctx, _default_mr_handle);
		vex::BindlessHandle normalHandle =
				_get_texture_handle(pbrMat ? pbrMat->get_normal_texture().get() : nullptr, ctx, _default_normal_handle);
		vex::BindlessHandle emissiveHandle = _get_texture_handle(
				pbrMat ? pbrMat->get_emissive_texture().get() : nullptr, ctx, vex::BindlessHandle {});

		// Build per-entity uniforms
		EntityUniforms entityUniforms;

		entityUniforms.model = entity.transform.to_matrix_with_scale();
		entityUniforms.normalMatrix = _compute_normal_matrix(entityUniforms.model);
		entityUniforms.baseColorFactor = pbrMat ? pbrMat->get_base_color_factor() : Color(1, 1, 1, 1);
		entityUniforms.metallicFactor = pbrMat ? pbrMat->get_metallic_factor() : 1.0f;
		entityUniforms.roughnessFactor = pbrMat ? pbrMat->get_roughness_factor() : 1.0f;
		entityUniforms.emissiveFactor = pbrMat ? pbrMat->get_emissive_factor() : Color(0, 0, 0, 1);
		entityUniforms.baseColorHandle = baseColorHandle;
		entityUniforms.metallicRoughnessHandle = metallicRoughnessHandle;
		entityUniforms.normalHandle = normalHandle;
		entityUniforms.emissiveHandle = emissiveHandle;

		std::vector<vex::TextureBinding> shadowMapBindings;
		for (auto& shadowMap : _shadow_maps) {
			shadowMapBindings.push_back(vex::TextureBinding { .texture = shadowMap });
		}

		// Draw
		vex::BufferBinding vertexBufferBinding {
			.buffer = meshBuffers.vertex_buffer,
			.strideByteSize = static_cast<uint32_t>(sizeof(Vertex)),
		};
		vex::BufferBinding indexBufferBinding {
			.buffer = meshBuffers.index_buffer,
			.strideByteSize = static_cast<uint32_t>(sizeof(uint32_t)),
		};

		std::array renderTargets = { vex::TextureBinding { .texture = backBuffer } };

		ctx.EnqueueDataUpload(_per_entity_uniform_buffer, to_bytes(entityUniforms));

		std::array<ResourceBinding, 3> bindings {
			BufferBinding { .buffer = _camera_uniform_buffer, .usage = BufferBindingUsage::ConstantBuffer },
			BufferBinding { .buffer = _per_entity_uniform_buffer, .usage = BufferBindingUsage::ConstantBuffer },
			BufferBinding { .buffer = _lights_structured_buffer,
					.usage = BufferBindingUsage::StructuredBuffer,
					.strideByteSize = 80 },
		};

		auto handles = graphics.GetBindlessHandles(bindings);

		ConstantBinding constant_bindings { std::span(handles) };
		ctx.DrawIndexed(_pbr_draw_desc,
				{
						.renderTargets = renderTargets,
						.depthStencil = vex::TextureBinding(depthTexture),
						.vertexBuffers = { &vertexBufferBinding, 1 },
						.indexBuffer = indexBufferBinding,
				},
				constant_bindings, meshBuffers.index_count);
	}
}

// Upload camera uniforms
void VexRenderer::_upload_camera_uniforms(const RenderCapture& capture, vex::CommandContext& ctx) {
	const auto& transform = capture.get_camera_transform();
	const auto& projection = capture.get_camera_projection();

	Matrix view = transform.to_matrix_no_scale().invert(); // World-to-camera
	Matrix proj = projection.get_matrix();
	Matrix viewProj = view * proj;

	CameraUniforms uniforms;

	uniforms.viewProj = viewProj;
	uniforms.cameraPos = transform.position;

	std::span<const std::byte> bytes = to_bytes(uniforms);
	ctx.EnqueueDataUpload(_camera_uniform_buffer, bytes);
}

// Upload lights buffer
void VexRenderer::_upload_lights_buffer(const RenderCapture& capture, vex::CommandContext& ctx) {
	const auto& lights = capture.get_lights();

	struct GPULight {
		uint32_t type;
		float _padding1[3];
		Vector3 position;
		float _padding2;
		Vector3 direction;
		float _padding3;
		Color color; // RGB + intensity in alpha
		float range;
		float spotAngleCos;
		uint32_t shadowMapIndex;
		float shadowBias;
	};

	std::vector<GPULight> gpuLights;
	for (size_t i = 0; i < lights.size(); ++i) {
		const auto& light = lights[i];
		GPULight gpuLight {};
		gpuLight.type = static_cast<uint32_t>(light.type);
		gpuLight.position = light.position;
		gpuLight.direction = light.direction;
		gpuLight.color = Color(light.color.x, light.color.y, light.color.z, light.intensity);
		gpuLight.range = light.range;
		gpuLight.spotAngleCos = std::cos(deg_to_rad(light.spot_angle));

		// Shadow map index
		auto it = _light_to_shadow_map_index.find(static_cast<uint32_t>(i));
		gpuLight.shadowMapIndex = (it != _light_to_shadow_map_index.end()) ? static_cast<uint32_t>(it->second)
																		   : static_cast<uint32_t>(-1);
		gpuLight.shadowBias = 0.005f;

		gpuLights.push_back(gpuLight);
	}

	if (!gpuLights.empty()) {
		ctx.EnqueueDataUpload(_lights_structured_buffer, std::as_bytes(std::span(gpuLights)));
	}
}

// Get or create mesh buffers
VexRenderer::MeshBuffers& VexRenderer::_get_or_create_mesh_buffers(
		const std::shared_ptr<TriangleMesh>& mesh, vex::CommandContext& ctx) {
	auto it = _mesh_cache.find(mesh);
	if (it != _mesh_cache.end()) {
		return it->second;
	}

	// Create vertex buffer
	const auto& vertices = mesh->get_vertices();
	vex::Buffer vb =
			graphics.CreateBuffer(vex::BufferDesc::CreateVertexBufferDesc("Mesh VB", sizeof(Vertex) * vertices.size()));
	ctx.EnqueueDataUpload(vb, std::as_bytes(std::span(vertices)));

	// Create index buffer
	const auto& indices = mesh->get_indices();
	vex::Buffer ib =
			graphics.CreateBuffer(vex::BufferDesc::CreateIndexBufferDesc("Mesh IB", sizeof(uint32_t) * indices.size()));
	ctx.EnqueueDataUpload(ib, std::as_bytes(std::span(indices)));

	MeshBuffers buffers { vb, ib, static_cast<uint32_t>(indices.size()) };
	_mesh_cache[mesh] = buffers;
	return _mesh_cache[mesh];
}

// Get or create texture
VexRenderer::TextureGPUData& VexRenderer::_get_or_create_texture(const Texture* texture, vex::CommandContext& ctx) {
	auto it = _texture_cache.find(texture);
	if (it != _texture_cache.end()) {
		return it->second;
	}

	// TODO: Implement actual texture loading from Texture resource
	// For now, return default white texture data
	TextureGPUData gpuData { _default_white_texture, _default_white_handle };
	_texture_cache[texture] = gpuData;
	return _texture_cache[texture];
}

// Get texture handle with fallback to default
vex::BindlessHandle VexRenderer::_get_texture_handle(
		const Texture* texture, vex::CommandContext& ctx, vex::BindlessHandle default_handle) {
	if (!texture) {
		return default_handle;
	}

	auto& gpuData = _get_or_create_texture(texture, ctx);
	return gpuData.bindless_handle;
}

// Compute light view-projection matrix
Matrix VexRenderer::_compute_light_view_proj(const RenderCapture::Light& light, const RenderCapture& capture) {
	if (light.type == RenderCapture::Light::Type::Directional) {
		// Orthographic projection covering scene
		Vector3 sceneCenter = _compute_scene_center(capture);
		float sceneRadius = _compute_scene_radius(capture, sceneCenter);

		Vector3 lightPos = sceneCenter - light.direction * (sceneRadius * 2.0f);
		Matrix view = Matrix::create_look_at(lightPos, sceneCenter, Vector3(0, 1, 0));
		Matrix proj = Matrix::create_orthographic(sceneRadius * 2.0f, sceneRadius * 2.0f, 0.1f, sceneRadius * 4.0f);
		return view * proj;
	}
	else if (light.type == RenderCapture::Light::Type::Spot) {
		// Perspective projection
		Matrix view = Matrix::create_look_at(light.position, light.position + light.direction, Vector3(0, 1, 0));
		float fov = light.spot_angle * 2.0f;
		Matrix proj = Matrix::create_perspective_field_of_view(deg_to_rad(fov), 1.0f, 0.1f, light.range);
		return view * proj;
	}

	// Point lights not implemented (requires cubemap)
	return Matrix::identity;
}

// Utility: compute scene center
Vector3 VexRenderer::_compute_scene_center(const RenderCapture& capture) {
	const auto& entities = capture.get_entities();
	if (entities.size() == 0)
		return Vector3::zero;

	Vector3 sum = Vector3::zero;
	for (const auto& entity : entities) {
		sum += entity.transform.position;
	}
	return sum / static_cast<float>(entities.size());
}

// Utility: compute scene radius
float VexRenderer::_compute_scene_radius(const RenderCapture& capture, const Vector3& center) {
	const auto& entities = capture.get_entities();
	float maxDist = 10.0f; // Minimum radius

	for (const auto& entity : entities) {
		float dist = Vector3::distance(center, entity.transform.position);
		// TODO: Add mesh bounding sphere radius
		maxDist = std::max(maxDist, dist + 10.0f); // Rough estimate
	}
	return maxDist;
}

// Utility: compute normal matrix
Matrix VexRenderer::_compute_normal_matrix(const Matrix& modelMatrix) {
	// Normal matrix is inverse-transpose of model matrix
	// For proper normal transformation with non-uniform scaling
	Matrix inv = modelMatrix.invert();
	return inv.transpose();
}

} //namespace feather
