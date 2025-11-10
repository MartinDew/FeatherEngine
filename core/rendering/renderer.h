#pragma once

#include <math/math_defs.h>

#include <array>

class SDL_Window;

namespace feather {

class Window;

class Renderer {
	friend class RenderingServer;

protected:
	Renderer();

	static constexpr std::array<Vector3, 8> example_cube;
	static constexpr std::array<uint32_t, 36> example_cube_indices;

	virtual void _on_resize() = 0;

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	virtual void _render_scene() = 0;
	virtual ~Renderer() = default;
};

} //namespace feather