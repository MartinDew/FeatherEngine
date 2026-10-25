export module feather.core:framework.assert;

import std;

export {
inline void fassert(bool condition, std::string message, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		std::println(std::cerr, "Assertion failed ({}:{}) : {}", loc.file_name(), loc.line(), message);
		std::terminate();
	}
}

inline void fassert(bool condition, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		std::println(std::cerr, "Assertion failed ({}:{})", loc.file_name(), loc.line());
		std::terminate();
	}
}
}
