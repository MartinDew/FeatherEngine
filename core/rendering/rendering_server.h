#pragma once

#include <memory>
#include <variant>

namespace feather {

class Renderer;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer;

public:
	RenderingServer();

	static RenderingServer* get() { return _instance; };

	void update(double dt);

	// Should change accessibility later
	template <class T>
	void use_renderer() {
		_renderer = std::make_unique<T>();
	}
};

} //namespace feather