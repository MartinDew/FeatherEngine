#pragma once
#include <rendering/renderer.h>
#include <array>

namespace feather {

class VexRenderer : public Renderer {
protected:
	void _render_scene(double dt) override;
};

} //namespace feather
