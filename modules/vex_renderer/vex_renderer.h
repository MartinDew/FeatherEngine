#pragma once
#include <core/rendering/renderer.h>
#include <array>

#include <Vex.h>

namespace feather {

class VexRenderer : public Renderer {
	vex::Texture depthTexture;
	vex::Texture uvGuideTexture;

	vex::Buffer vertexBuffer;
	vex::Buffer indexBuffer;

	vex::Graphics graphics;

	static vex::PlatformWindowHandle _create_vex_window(Window& window);

protected:
	void _render_scene() override;
	void _on_resize() override;

public:
	VexRenderer();
	~VexRenderer() override;
};

} //namespace feather
