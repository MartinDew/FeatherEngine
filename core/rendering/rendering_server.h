#pragma once

#include "renderer.h"

#include <memory>

namespace feather {

class Renderer;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer = nullptr;

public:
	RenderingServer();

	static RenderingServer* get() { return _instance; };

	void update(double dt);

	// Should change accessibility later
	template <class T> void use_renderer() { _renderer = std::make_unique<T>(); }
};

} //namespace feather