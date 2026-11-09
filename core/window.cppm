module;

#include <SDL3/SDL_events.h>

export module feather.core:window;

import std;
import feather.framework;
import :notification;

export namespace feather {

class FEATHER_CORE_API Window {
public:
	struct WindowProperties {
		int width;
		int height;
		int x;
		int y;
	};

	enum class FullscreenMode {
		WINDOWED,
		FULLSCREEN,
		BORDERLESS
	};

private:
	SDL_Window* _internal_window = nullptr;
	SDL_Event _internal_event;

	WindowProperties _properties;
	FullscreenMode _fullscreen_mode;

	using NotificationDelegate = Delegate<>;
	std::array<NotificationDelegate, std::to_underlying(Notification::COUNT)> _notification_listeners;

	void _on_resize();
	void _on_move();

public:
	Window();

	// Resolve events
	bool update();

	inline SDL_Window* get_internal_window() const { return _internal_window; }

	const WindowProperties& properties = _properties;
	const FullscreenMode& fullscreen_mode = _fullscreen_mode;

	void set_fullscreen_mode(const FullscreenMode mode) const;
	void register_notification(Notification notification, const std::function<void()>& delegate);
};

} //namespace feather
