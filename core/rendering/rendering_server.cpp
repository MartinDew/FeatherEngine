#include "rendering_server.h"
#include "main/engine.h"
#include "main/notification.h"
#include "renderer.h"

#include <framework/assert.h>
#include <main/class_db.h>
#include <main/launch_settings.h>
#include <framework/static_string.hpp>

#include <string_view>

namespace feather {

RenderingServer* RenderingServer::_instance = nullptr;

void RenderingServer::_run() { _render_thread = std::jthread(bind_method(&RenderingServer::_render_function, this)); }

void RenderingServer::_render_function() {
	while (true) {
		if (_render_thread.get_stop_token().stop_requested()) {
			break;
		}

		std::unique_lock lock(_render_lock);
		_render_cv.wait(lock);

		if (_needs_resize) {
			_renderer->_on_resize();
			_needs_resize = false;
		}

		// Lockless read of RenderCapture
		int read_idx = 1 - _write_index.load(std::memory_order_acquire);
		_renderer->_render_scene(_capture_buffers[read_idx]);
	}
}

RenderingServer::RenderingServer() {
	fassert(!_instance);

	_instance = this;
}

void RenderingServer::init() {
	_renderer = ClassDB::create_object<Renderer>(LaunchSettings::get().renderer.Get());

	Engine::get().get_main_window().register_notification(
			Notification::WINDOW_RESIZED, [this] { _needs_resize = true; });

	_run();
};

void RenderingServer::update(double dt) {
	fassert(_renderer.get(), "no renderer set");

	_render_cv.notify_all();
}

void RenderingServer::set_render_capture(const RenderCapture& capture) {
	// Lockless write: copy to write buffer, then swap
	int write_idx = _write_index.load(std::memory_order_relaxed);
	_capture_buffers[write_idx] = capture; // CowVector makes this cheap
	_write_index.store(1 - write_idx, std::memory_order_release);
}

void RenderingServer::use_renderer(std::string_view name) {
	_renderer = ClassDB::create_object<Renderer>(name);
	fassert(_renderer.get(), std::format("Failed to create renderer of type {}", name));
}

RenderingServer* RenderingServer::get() {
	fassert(_instance, "instance not yet initialized for RenderingServer");
	return _instance;
}

} //namespace feather