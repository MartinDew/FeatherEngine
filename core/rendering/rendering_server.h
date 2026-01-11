#pragma once

#include "framework/spinlock.h"
#include "main/launch_settings.h"
#include "renderer.h"

#include <main/engine_settings.h>

#include <condition_variable>
#include <memory>
#include <thread>

namespace feather {

class Renderer;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer = nullptr;

	std::jthread _render_thread;
	spinlock _render_lock;
	std::condition_variable_any _render_cv;

	void _run();
	void _render_function();

	bool _needs_resize = false;

public:
	RenderingServer();

	static RenderingServer* get();

	void init();
	void update(double dt);

	// Should change accessibility later
	template <class T> void use_renderer() { _renderer = std::make_unique<T>(); }

	void use_renderer(std::string_view name);
};

} //namespace feather