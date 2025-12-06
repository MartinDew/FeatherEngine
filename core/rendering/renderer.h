#pragma once

#include "triangle_mesh.h"

#include <main/engine_settings.h>
#include <math/math_defs.h>

#include <array>
#include <vector>

class SDL_Window;

namespace feather {

class Window;

class Renderer {
	friend class RenderingServer;

protected:
	Renderer();

	static TriangleMesh get_example_cube();

	virtual void _on_resize() = 0;

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	virtual void _render_scene() = 0;
	virtual ~Renderer() = default;
};

} // namespace feather