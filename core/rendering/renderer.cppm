module;

#include <SDL3/SDL_video.h>
#include <framework/export_defs.h>
#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "renderer.gen.h"
#endif

export module feather.core:rendering.renderer;

import :rendering.mesh_data;
import :rendering.render_scene;
import :framework.reflected;
import :main.engine_settings;
import :main.class_db;

export namespace feather {

class Shader;
class Window;

class FEATHER_API Renderer : public Reflected {
	FCLASS();
	friend class RenderingServer;

protected:
	Renderer();

	virtual void _on_resize() = 0;
	virtual void _compile_shader(Shader& shader) {}

	static SDL_Window* _extract_internal_window(Window& window);

	Window* _window;

public:
	[[method]]
	virtual void _render_scene(RenderScene capture) = 0;
	~Renderer() override = default;
};

} // namespace feather
