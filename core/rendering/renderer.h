#pragma once
namespace feather {

class Renderer {
public:
	virtual ~Renderer() = default;

private:
	friend class RenderingServer;

protected:
	Renderer();

	virtual void _render_scene() = 0;
	virtual void _on_resize() = 0;
};

} //namespace feather