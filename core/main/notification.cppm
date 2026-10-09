module;

#include <map>

export module feather.core:main.notification;

export namespace feather {

enum class Notification : uint32_t {
	NONE = 0,
	WINDOW_SHOWN,
	WINDOW_RESIZED,
	COUNT
};

} //namespace feather
