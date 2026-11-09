module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "null_renderer.gen.h"
#endif

export module feather.rendering:null_renderer;

import std;
import feather.framework;
import :renderer;
import feather.reflection;

export namespace feather {

// Renderer used in headless mode. Satisfies RenderingServer's contract without
// creating a GPU device, so a dedicated server needs neither a display nor a
// graphical driver.
class FEATHER_RENDERING_API NullRenderer final : public Renderer {
	FCLASS();

protected:
	inline void _render_scene(RenderScene capture) override {}
	inline void _on_resize() override {}
};

} //namespace feather
