#pragma once

#include "renderer.h"

#include <main/engine_settings.h>

#include <memory>

namespace feather {

class Renderer;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer = nullptr;

public:
	RenderingServer();

	static RenderingServer* get();

	void update(double dt);

	// Should change accessibility later
	template <class T> void use_renderer() { _renderer = std::make_unique<T>(); }

	void use_renderer(std::string_view name);
};

} //namespace feather