#pragma once

#include <math/math_defs.h>

#include <array>

#include <main/engine_settings.h>

class SDL_Window;

namespace feather {

class Window;

// temp

struct Vertex {
	Vector3 position;
	std::array<real_t, 2> uv;

	Vertex() = default;
	constexpr Vertex(real_t x, real_t y, real_t z, real_t u, real_t v) : position(x, y, z), uv { u, v } {}
};

class Renderer {
	friend class RenderingServer;

protected:
	Renderer();

	static constexpr std::array<Vertex, 8> example_cube {
		// Front face
		Vertex { -1.f, -1.f, 1.f, 0, 0 }, // 0: bottom-left
		Vertex { 1.f, -1.f, 1.f, 1, 0 }, // 1: bottom-right
		Vertex { 1.f, 1.f, 1.f, 1, 1 }, // 2: top-right
		Vertex { -1.f, 1.f, 1.f, 0, 1 }, // 3: top-left
		// Back face
		Vertex { -1.f, -1.f, -1.f, 1, 0 }, // 4: bottom-left
		Vertex { 1.f, -1.f, -1.f, 0, 0 }, // 5: bottom-right
		Vertex { 1.f, 1.f, -1.f, 0, 1 }, // 6: top-right
		Vertex { -1.f, 1.f, -1.f, 1, 1 }, // 7: top-left
	};

	static constexpr std::array<uint32_t, 36> example_cube_indices { // Front face
		0, 1, 2, 2, 3, 0,
		// Back face
		4, 6, 5, 6, 4, 7,
		// Left face
		4, 0, 3, 3, 7, 4,
		// Right face
		1, 5, 6, 6, 2, 1,
		// Top face
		3, 2, 6, 6, 7, 3,
		// Bottom face
		4, 5, 1, 1, 0, 4
	};

	virtual void _on_resize() = 0;

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	virtual void _render_scene() = 0;
	virtual ~Renderer() = default;
};

// Engine setting for which renderer to use
inline static ResolverSetting<StaticString, std::unique_ptr<Renderer>> renderer_setting { "renderer", "sdl"_ss };

} // namespace feather