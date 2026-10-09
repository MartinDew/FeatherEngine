module;

#include <array>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

export module feather.core:rendering.rendering_server;

import :framework.spinlock;
import :main.launch_settings;
import :rendering.render_scene;
import :rendering.renderer;
import :main.engine_settings;

export namespace feather {

class Renderer;
class Shader;

class RenderingServer {
	static RenderingServer* _instance;

	std::unique_ptr<Renderer> _renderer = nullptr;

	std::array<RenderScene, 2> _buffers;
	std::atomic<int> _write_idx { 0 };
	std::atomic<bool> _dirty { false };
	spinlock _write_lock;
	std::mutex _wait_mutex;
	std::condition_variable _wait_cv;

	std::jthread _render_thread;

	void _run();
	void _render_function();

	std::atomic<bool> _needs_resize { false };

public:
	RenderingServer();
	~RenderingServer();

	static RenderingServer* get();

	void init();
	void update(double dt) const;
	void stop();

	void begin_scene_frame();
	void set_camera_transform(const Transform& transform);
	void set_camera_projection(const Projection& projection);
	void set_environment(const RenderScene::EnvironmentSettings& env);
	void add_entity(const RenderScene::EntityRender& entity);
	void add_light(const Light& light);
	void commit_scene_frame();

	template <class T> void use_renderer() { _renderer = std::make_unique<T>(); }
	void use_renderer(std::string_view name);

	void compile_shader(const std::shared_ptr<Shader>& shader);
};

} //namespace feather
