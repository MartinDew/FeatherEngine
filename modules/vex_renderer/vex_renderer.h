#pragma once
#include <rendering/renderer.h>
#include <array>

#include <Vex.h>

namespace feather {

class VexRenderer : public Renderer {
	vex::Texture depthTexture;
	vex::Texture uvGuideTexture;

	vex::Buffer vertexBuffer;
	vex::Buffer indexBuffer;

protected:
	VexRenderer();

	void _render_scene() override;
	void _on_resize() override;
};

} //namespace feather
