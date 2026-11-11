module;

#include <SDL3/SDL_video.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "renderer.gen.h"
#endif

export module feather.rendering:renderer;

import std;
import feather.framework;
import feather.reflection;
import :render_scene;
import feather.core;
import feather.resources;

export namespace feather {

class FEATHER_RENDERING_API Renderer : public Reflected {
	FCLASS();
	friend class RenderingServer;

protected:
	Renderer();

	virtual void _on_resize() = 0;
	inline virtual void _compile_shader(Shader& shader) {}

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	[[method]]
	virtual void _render_scene(RenderScene capture) = 0;
	~Renderer() override = default;
};

} // namespace feather
