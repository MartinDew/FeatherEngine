#pragma once

#include <core/framework/reflection_macros.h>
#include <core/rendering/renderer.h>
#include <array>

#include <Vex.h>

namespace feather {

class VexRenderer : public Renderer {
	FCLASS(VexRenderer, Renderer);

	vex::Texture depthTexture;
	vex::Texture uvGuideTexture;

	vex::Buffer vertexBuffer;
	vex::Buffer indexBuffer;

	vex::Graphics graphics;

	static vex::PlatformWindowHandle _create_vex_window(Window& window);

	size_t test_variable = 8;

protected:
	void _render_scene() override;
	void _on_resize() override;

	static void _bind_members();

public:
	VexRenderer();
};

} //namespace feather
