#pragma once

#include "render_capture.h"
#include "triangle_mesh.h"

#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#include <main/engine_settings.h>
#include <math/math_defs.h>

class SDL_Window;

namespace feather {

class Window;

class Renderer : public Reflected {
	FCLASS(Renderer, Reflected)
	friend class RenderingServer;

protected:
	Renderer();

	static TriangleMesh get_example_cube();

	virtual void _on_resize() = 0;

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

	static void _bind_members();

public:
	virtual void _render_scene(RenderCapture capture) = 0;
	virtual ~Renderer() = default;
};

} // namespace feather