#pragma once

#include "notification.h"
#include <framework/delegate.h>
#include <framework/reflection_macros.h>

struct SDL_Window;

namespace feather {

class Window final : public Reflected {
	FCLASS(Window, Reflected);
	friend class Renderer;

public:
	enum class FullscreenMode : uint8_t {
		WINDOWED,
		FULLSCREEN,
		BORDERLESS
	};

protected:
	static void _bind_members();

private:
	int _width;
	int _height;
	int _x;
	int _y;
	SDL_Window* _internal_window = nullptr;
	FullscreenMode _fullscreen_mode;

	using NotificationDelegate = Delegate<>;
	std::array<NotificationDelegate, std::to_underlying(Notification::COUNT)> _notification_listeners;

	void _on_resize();
	void _on_move();

public:
	Window();

	// Resolve events
	bool update();

	[[nodiscard]] int get_x() const { return _x; }
	[[nodiscard]] int get_y() const { return _y; }
	[[nodiscard]] int get_width() const { return _width; }
	[[nodiscard]] int get_height() const { return _height; }
	[[nodiscard]] FullscreenMode get_fullscreen_mode() const { return _fullscreen_mode; }

	void set_fullscreen_mode(const FullscreenMode mode) const;
	void register_notification(Notification notification, const std::function<void()>& delegate);
};

} //namespace feather