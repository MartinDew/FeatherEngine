#include "rendering_server.h"
#include "renderer.h"
#include <main/engine.h>
#include <main/notification.h>

#include <framework/assert.h>
#include <framework/variant.h>
#include <main/class_db.h>
#include <main/launch_settings.h>
#include <framework/static_string.hpp>

#include <string_view>

namespace feather {

RenderingServer* RenderingServer::_instance = nullptr;

void RenderingServer::_run() {
	_render_thread = std::jthread(bind_method(&RenderingServer::_render_function, this));
}

void RenderingServer::_render_function() {
	while (true) {
		if (_render_thread.get_stop_token().stop_requested()) {
			break;
		}

		{
			std::unique_lock lock(_render_lock);
			_render_cv.wait(lock, [this] {
				int read_idx = 1 - _write_index.load(std::memory_order_acquire);
				uint64_t current_frame = _capture_buffers[read_idx].get_frame_index();
				uint64_t last_frame = _last_rendered_frame.load(std::memory_order_relaxed);
				return current_frame != last_frame;
			});
		}

		if (_needs_resize) {
			_renderer->_on_resize();
			_needs_resize = false;
		}

		// Lockless read of RenderCapture
		int read_idx = 1 - _write_index.load(std::memory_order_acquire);
		_render_scene_lock.lock();
		const RenderScene render_scene = _capture_buffers[read_idx];
		_render_scene_lock.unlock();

		_renderer->_render_scene(render_scene);
		_last_rendered_frame.store(render_scene.get_frame_index(), std::memory_order_relaxed);
	}
}

RenderingServer::RenderingServer() {
	fassert(!_instance);

	_instance = this;
}

void RenderingServer::init() {
	_renderer = ClassDB::create_object<Renderer>(LaunchSettings::get().renderer.Get());

	Engine::get().get_main_window().register_notification(Notification::WINDOW_RESIZED, [this] {
		_needs_resize = true;
	});

	if (!LaunchSettings::get().force_single_thread.Get())
		_run();
};

void RenderingServer::update(double dt) const {
	fassert(_renderer.get(), "no renderer set");
}

void RenderingServer::stop() {
	_render_thread.request_stop();
	if (_render_thread.joinable())
		_render_thread.join();
}

void RenderingServer::set_render_capture(const RenderScene& capture) {
	std::unique_lock<spinlock> lock { _render_scene_lock };
	// Lockless write: copy to write buffer, then swap
	int write_idx = _write_index.load(std::memory_order_relaxed);
	_capture_buffers[write_idx] = std::move(capture); // CowVector makes this cheap
	_write_index.store(1 - write_idx, std::memory_order_release);

	_render_cv.notify_all();

	if (LaunchSettings::get().force_single_thread.Get()) {
		_renderer->_render_scene(capture);
	}
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