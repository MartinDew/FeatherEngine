#pragma once

#include <format>
#include <source_location>

template <class... T>
void fassert(bool condition, const char* message, const T&... args, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		auto formatted_message = std::vformat(message, std::make_format_args(args...));
		throw std::runtime_error(std::format("Assertion failed ({}:{}) : {}", loc.file_name(), loc.line(), formatted_message));
	}
}

inline void fassert(bool condition, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		throw std::runtime_error(std::format("Assertion failed ({}:{})", loc.file_name(), loc.line()));
	}
}