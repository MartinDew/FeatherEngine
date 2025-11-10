#include "window.h"

#include "engine.h"
#include "notification.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_video.h>

#include <cassert>
#include <iostream>

namespace feather {

namespace {

constexpr Notification to_notification(const SDL_Event& event) {
	switch (event.type) {
	case SDL_EVENT_WINDOW_SHOWN:
		return Notification::WINDOW_SHOWN;
	case SDL_EVENT_WINDOW_RESIZED:
		return Notification::WINDOW_RESIZED;
	default:
		return Notification::NONE;
	}
}

} //namespace

Window::Window() :
		_internal_event(), _fullscreen_mode() {
	if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
		std::cerr << SDL_GetError() << std::endl;
		assert(false);
	}

	const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());

	if (!dm) {
		std::cerr << SDL_GetError() << std::endl;
		assert(false);
	}

	_properties.width = dm->w / 2;
	_properties.height = dm->h / 2;

	auto window_flags = SDL_WINDOW_MOUSE_CAPTURE;

	if (Engine::is_editor()) {
		window_flags |= SDL_WINDOW_RESIZABLE;
	}

	_internal_window = SDL_CreateWindow("Feather", _properties.width, _properties.height, window_flags);

	SDL_GetWindowPosition(_internal_window, &_properties.x, &_properties.y);
}

void Window::_on_resize() {
	SDL_GetWindowSize(_internal_window, &_properties.width, &_properties.height);
	SDL_LogDebug(0, "Window resized to %dx%d", _properties.width, _properties.height);
}

void Window::_on_move() {
	SDL_GetWindowPosition(_internal_window, &_properties.x, &_properties.y);
	SDL_LogDebug(0, "Window moved to %d,%d", _properties.x, _properties.y);
}

void Window::set_fullscreen_mode(const FullscreenMode mode) const {
	switch (mode) {
	case FullscreenMode::WINDOWED:
		SDL_SetWindowFullscreen(_internal_window, false);
		break;
	case FullscreenMode::FULLSCREEN:
		SDL_SetWindowFullscreen(_internal_window, true);
		break;
	case FullscreenMode::BORDERLESS: {
		SDL_SetWindowFullscreen(_internal_window, false);
		SDL_SetWindowBordered(_internal_window, false);

		// get display size
		const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
		if (!dm) {
			std::cerr << SDL_GetError() << std::endl;
			fassert(false);
		}

		SDL_SetWindowSize(_internal_window, dm->w, dm->h);
		SDL_SetWindowPosition(_internal_window, 0, 0);
		SDL_SetWindowResizable(_internal_window, false);
		break;
	}
	default:
		std::unreachable();
		break;
	}
}

void Window::register_notification(Notification notification, const std::function<void()>& delegate) {
	_notification_listeners[std::to_underlying(notification)].subscribe(delegate);
}

bool Window::update() {
	while (SDL_PollEvent(&_internal_event)) {
		switch (_internal_event.type) {
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			SDL_LogDebug(0, "Close window requested");
			return false;
		case SDL_EVENT_WINDOW_RESIZED:
			_on_resize();
			break;
		}

		auto notification = to_notification(_internal_event);
		_notification_listeners[std::to_underlying(notification)].execute();
	}

	return true;
}

} //namespace feather
