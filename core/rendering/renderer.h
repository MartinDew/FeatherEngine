#pragma once

#include <math/math_defs.h>

#include <array>

class SDL_Window;

namespace feather {

class Window;

// temp

struct Vertex {
	Vector3 position;
	std::array<real_t, 2> uv;

	Vertex() = default;
	constexpr Vertex(real_t x, real_t y, real_t z, real_t u, real_t v) : position(x, y, z), uv{ u, v } {}
};

class Renderer {
	friend class RenderingServer;

protected:
	Renderer();

	static constexpr std::array<Vertex, 8> example_cube{
		Vertex(-1.0f, -1.0f, -1.0f, 0.f, 0.f),
		Vertex(1.0f, -1.0f, -1.0f, 1.f, 0.f),
		Vertex(1.0f, 1.0f, -1.0f, 1.f, 1.f),
		Vertex(-1.0f, 1.0f, -1.0f, 0.f, 1.f),
		Vertex(-1.0f, -1.0f, 1.0f, 0.f, 0.f),
		Vertex(1.0f, -1.0f, 1.0f, 1.f, 0.f),
		Vertex(1.0f, 1.0f, 1.0f, 1.f, 1.f),
		Vertex(-1.0f, 1.0f, 1.0f, 0.f, 1.f),
	};

	static constexpr std::array<uint32_t, 36> example_cube_indices{
		0, 1, 2, 2, 3, 0, // Back face
		4, 5, 6, 6, 7, 4, // Front face
		0, 4, 7, 7, 3, 0, // Left face
		1, 5, 6, 6, 2, 1, // Right face
		3, 2, 6, 6, 7, 3, // Top face
		0, 1, 5, 5, 4, 0 // Bottom face
	};

	virtual void _on_resize() = 0;

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	virtual void _render_scene() = 0;
	virtual ~Renderer() = default;
};

} //namespace feather