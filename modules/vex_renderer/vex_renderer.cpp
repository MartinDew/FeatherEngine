#include "vex_renderer.h"

#include "main/engine_settings.h"
#include <core/main/engine.h>
#include <core/main/window.h>
#include <core/math/math_defs.h>

#include <cstdint>
#include <ctime>
#include <fstream>
#include <ostream>

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
} //namespace feather

void VexRenderer::_bind_members() {}

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

	// TODO Setup error handling

	// Depth texture
	depthTexture = graphics.CreateTexture({
			.name = "Depth Texture",
			.type = vex::TextureType::Texture2D,
			.format = vex::TextureFormat::D32_FLOAT,
			.width = (width),
			.height = (height),
			.usage = vex::TextureUsage::DepthStencil,
			.clearValue =
					vex::TextureClearValue {
							.flags = vex::TextureClear::ClearDepth,
							.depth = 0,
					},
	});

	const auto& example_cube = get_example_cube();

	// Vertex buffer
	vertexBuffer = graphics.CreateBuffer(vex::BufferDesc::CreateVertexBufferDesc(
			"Vertex Buffer", sizeof(Vertex) * example_cube.get_vertices().size()));
	// Index buffer
	indexBuffer = graphics.CreateBuffer(vex::BufferDesc::CreateIndexBufferDesc(
			"Index Buffer", sizeof(uint32_t) * example_cube.get_indices().size()));

	// Immediate submission means the commands are instantly submitted upon destruction.
	vex::CommandContext ctx = graphics.CreateCommandContext(vex::QueueType::Graphics);

	ctx.EnqueueDataUpload(vertexBuffer, std::as_bytes(std::span(example_cube.get_vertices())));
	ctx.EnqueueDataUpload(indexBuffer, std::as_bytes(std::span(example_cube.get_indices())));

	// Use the loaded image for mip index 0.
	// const std::filesystem::path uvImagePath = ExamplesDir / "uv-guide.png";
	// vex::i32 width, height, channels;
	// void* imageData = stbi_load(uvImagePath.string().c_str(), &width, &height, &channels, 4);
	// VEX_ASSERT(imageData != nullptr);

	// Vex requires that the upload data for textures be tightly packed together! This shouldn't be an issue as most
	// file formats tightly pack data to avoid wasting space with padding.
	std::vector<vex::u8> fullImageData;
	fullImageData.reserve(width * height /** channels*/);
	// std::copy_n(static_cast<vex::u8*>(imageData), width * height /** channels*/, std::back_inserter(fullImageData));

	uvGuideTexture = graphics.CreateTexture({ .name = "UV Guide",
			.type = vex::TextureType::Texture2D,
			.format = vex::TextureFormat::RGBA8_UNORM,
			.width = static_cast<uint32_t>(width),
			.height = static_cast<uint32_t>(height),
			.depthOrSliceCount = 1,
			.mips = 0, // 0 means max mips (down to 1x1)
			.usage = vex::TextureUsage::ShaderRead | vex::TextureUsage::ShaderReadWrite });

	// Upload only to the first mip
	// ctx.EnqueueDataUpload(uvGuideTexture,
	// 		std::as_bytes(std::span(fullImageData.begin(), fullImageData.begin() + width * height /** channels*/)),
	// 		vex::TextureRegion::SingleMip(0));

	// Fill in all mips using the first one.
	// ctx.GenerateMips(uvGuideTexture);

	// The texture will now only be used as a read-only shader resource. Avoids having to place a barrier later on.
	// We use PixelShader sync since it will only be used there.
	ctx.Barrier(uvGuideTexture, vex::RHIBarrierSync::PixelShader, vex::RHIBarrierAccess::ShaderRead,
			vex::RHITextureLayout::ShaderResource);

	// stbi_image_free(imageData);

	std::array samplers {
		vex::TextureSampler::CreateSampler(vex::FilterMode::Linear, vex::AddressMode::Clamp),
		vex::TextureSampler::CreateSampler(vex::FilterMode::Point, vex::AddressMode::Clamp),
	};
	graphics.SetSamplers(samplers);

	graphics.Submit(ctx);
}

void VexRenderer::_render_scene() {
	// Scoped command context will submit commands automatically upon destruction.

	const auto& example_cube = get_example_cube();

	{
		auto ctx = graphics.CreateCommandContext(vex::QueueType::Graphics);

		ctx.SetScissor(0, 0, _window->properties.width, _window->properties.height);
		ctx.SetViewport(0, 0, _window->properties.width, _window->properties.height);

		// Clear backbuffer.
		vex::TextureClearValue clearValue { .flags = vex::TextureClear::ClearColor, .color = { 0.2f, 0.2f, 0.2f, 1 } };
		ctx.ClearTexture(
				vex::TextureBinding {
						.texture = graphics.GetCurrentPresentTexture(),
				},
				clearValue);

		// Clear depth texture.
		ctx.ClearTexture({ .texture = depthTexture });

		vex::VertexInputLayout vertexLayout{
		.attributes = {
				{
						.semanticName = "POSITION",
						.semanticIndex = 0,
						.binding = 0,
						.format = vex::TextureFormat::RGB32_FLOAT,
						.offset = 0,
				},
				{
						.semanticName = "TEXCOORD",
						.semanticIndex = 0,
						.binding = 0,
						.format = vex::TextureFormat::RG32_FLOAT,
						.offset = sizeof(float) * 3,
				} },
		.bindings = {
				{
						.binding = 0,
						.strideByteSize = static_cast<uint32_t>(20),
						.inputRate = vex::VertexInputLayout::InputRate::PerVertex,
				},
		},
	};

		vex::DepthStencilState depthStencilState {
			.depthTestEnabled = true,
			.depthWriteEnabled = true,
			.depthCompareOp = vex::CompareOp::GreaterEqual,
		};

		// Setup our draw call's description...
		vex::DrawDesc hlslDrawDesc{
			.vertexShader = {
					.path = "shaders/example_cube.hlsl",
					.entryPoint = "VSMain",
					.type = vex::ShaderType::VertexShader,
			},
			.pixelShader = {
					.path = "shaders/example_cube.hlsl",
					.entryPoint = "PSMain",
					.type = vex::ShaderType::PixelShader,
			},
			.vertexInputLayout = vertexLayout,
			.depthStencilState = depthStencilState,
		};
#if VEX_SLANG
		vex::DrawDesc slangDrawDesc{
				.vertexShader = {
						.path = "shaders/example_cube.slang",
						.entryPoint = "VSMain",
						.type = vex::ShaderType::VertexShader,
				},
				.pixelShader = {
						.path = "shaders/example_cube.slang",
						.entryPoint = "PSMain",
						.type = vex::ShaderType::PixelShader,
				},
				.vertexInputLayout = vertexLayout,
				.depthStencilState = depthStencilState,
			};
#endif
		// ...and resources.
		vex::BufferBinding vertexBufferBinding {
			.buffer = vertexBuffer,
			.strideByteSize = static_cast<uint32_t>(sizeof(Vertex)),
		};
		vex::BufferBinding indexBufferBinding {
			.buffer = indexBuffer,
			.strideByteSize = static_cast<uint32_t>(sizeof(uint32_t)),
		};

		// Setup our rendering pass.
		std::array renderTargets = { vex::TextureBinding {
				.texture = graphics.GetCurrentPresentTexture(),
		} };

		// Usually you'd have to transition the uvGuideTexture (since we're using it bindless-ly), but since we
		// already transitioned it to RHITextureState::ShaderResource after the texture upload we don't have to!
		vex::BindlessHandle uvGuideHandle = graphics.GetBindlessHandle(
				vex::TextureBinding { .texture = uvGuideTexture, .usage = vex::TextureBindingUsage::ShaderRead });

		struct UniformData {
			float currentTime;
			vex::BindlessHandle uvGuideHandle;
		};

		static float accum = 0;
		accum += Engine::get().get_current_delta_time();
		{
			VEX_GPU_SCOPED_EVENT(ctx, "HLSL Cube");
			ctx.DrawIndexed(hlslDrawDesc,
					{
							.renderTargets = renderTargets,
							.depthStencil = vex::TextureBinding(depthTexture),
							.vertexBuffers = { &vertexBufferBinding, 1 },
							.indexBuffer = indexBufferBinding,
					},

					vex::ConstantBinding(UniformData { .currentTime = accum, .uvGuideHandle = uvGuideHandle }),
					example_cube.get_indices().size());
		}

#if VEX_SLANG
		{
			VEX_GPU_SCOPED_EVENT(ctx, "Slang Cube");
			ctx.DrawIndexed(slangDrawDesc,
					{
							.renderTargets = renderTargets,
							.depthStencil = vex::TextureBinding(depthTexture),
							.vertexBuffers = { &vertexBufferBinding, 1 },
							.indexBuffer = indexBufferBinding,
					},
					vex::ConstantBinding(UniformData { static_cast<float>(accum), uvGuideHandle }),
					example_cube.get_indices().size());
		}
#endif

		graphics.Submit(ctx);
	}

	graphics.Present(_window->fullscreen_mode == Window::FullscreenMode::FULLSCREEN);
}

void VexRenderer::_on_resize() {
	auto width = _window->properties.width;
	auto height = _window->properties.height;
	if (width == 0 || height == 0) {
		return;
	}

	graphics.DestroyTexture(depthTexture);

	depthTexture = graphics.CreateTexture({
			.name = "Depth Texture",
			.type = vex::TextureType::Texture2D,
			.format = vex::TextureFormat::D32_FLOAT,
			.width = static_cast<vex::u32>(width),
			.height = static_cast<vex::u32>(height),
			.usage = vex::TextureUsage::DepthStencil,
			.clearValue =
					vex::TextureClearValue {
							.flags = vex::TextureClear::ClearDepth,
							.depth = 0,
					},
	});

	graphics.OnWindowResized(width, height);
}

} //namespace feather