#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <source_location>

template <class... T>
void fassert(bool condition, const char* message, const T&... args,
		std::source_location loc = std::source_location::current()) {
	if (!condition) {
		auto formatted_message = std::vformat(message, std::make_format_args(args...));
		std::cerr << std::format("Assertion failed ({}:{}) : {}", loc.file_name(), loc.line(), formatted_message);
		std::exit;
	}
}

inline void fassert(bool condition, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		std::cerr << std::format("Assertion failed ({}:{})", loc.file_name(), loc.line());
		std::exit;
	}
}