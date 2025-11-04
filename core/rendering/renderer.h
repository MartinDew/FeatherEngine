#pragma once
namespace feather {

class Renderer {
	friend class RenderingServer;

protected:
	virtual void _render_scene(double dt) = 0;
};

} //namespace feather