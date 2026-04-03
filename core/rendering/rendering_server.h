#pragma once

#include "framework/spinlock.h"
#include "main/launch_settings.h"
#include "render_scene.h"
#include "renderer.h"

#include <main/engine_settings.h>

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <thread>

namespace feather {

class Renderer;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer = nullptr;

	// Lockless RenderCapture passing via double-buffering
	std::array<RenderScene, 2> _capture_buffers;
	std::atomic<int> _write_index { 0 };
	std::atomic<uint64_t> _last_rendered_frame { 0 };

	std::jthread _render_thread;
	spinlock _render_lock;
	spinlock _render_scene_lock;
	std::condition_variable_any _render_cv;

	void _run();
	void _render_function();

	bool _needs_resize = false;

public:
	RenderingServer();

	static RenderingServer* get();

	void init();
	void update(double dt) const;
	void stop();
	// Set render capture (lockless, called from main thread)
	void set_render_capture(const RenderScene& capture);

	// Should change accessibility later
	template <class T> void use_renderer() { _renderer = std::make_unique<T>(); }

	void use_renderer(std::string_view name);
};

} //namespace feather