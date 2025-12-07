#pragma once

#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <source_location>
#include <stdexcept>

template <class... T>
void fassert(bool condition, std::string message, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		throw std::runtime_error(std::format("Assertion failed ({}:{}) : {}", loc.file_name(), loc.line(), message));
	}
}

inline void fassert(bool condition, std::source_location loc = std::source_location::current()) {
	if (!condition) {
		throw std::runtime_error(std::format("Assertion failed ({}:{})", loc.file_name(), loc.line()));
	}
}