#pragma once

#include <cstddef>
#include <span>
#include <type_traits>

using Byte = std::byte;

// Convert a reference to object into a read-only Byte span
template <class T>
std::span<const Byte, sizeof(T)> to_bytes(const T& obj) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");

	return std::span<const Byte, sizeof(T)>(reinterpret_cast<const Byte*>(&obj), sizeof(T));
}

// Mutable version for non-const references
template <class T>
std::span<Byte, sizeof(T)> to_bytes(T& obj) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");

	return std::span<Byte, sizeof(T)>(reinterpret_cast<Byte*>(&obj), sizeof(T));
}

// Pointer overloads with null checking
template <class T>
std::span<const Byte, sizeof(T)> to_bytes(const T* ptr) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");

	if (!ptr) {
		return {};
	}

	return std::span<const Byte, sizeof(T)>(reinterpret_cast<const Byte*>(ptr), sizeof(T));
}

template <class T>
std::span<Byte, sizeof(T)> to_bytes(T* ptr) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");

	if (!ptr) {
		return {};
	}

	return std::span<Byte, sizeof(T)>(reinterpret_cast<Byte*>(ptr), sizeof(T));
}

// Array overloads
template <class T, std::size_t N>
std::span<const Byte, sizeof(T) * N> to_bytes(const T (&arr)[N]) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");
	return std::span<const Byte, sizeof(T) * N>(reinterpret_cast<const Byte*>(arr), sizeof(T) * N);
}

template <class T, std::size_t N>
std::span<Byte, sizeof(T) * N> to_bytes(T (&arr)[N]) {
	static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable for Byte conversion");

	return std::span<Byte, sizeof(T) * N>(reinterpret_cast<Byte*>(arr), sizeof(T) * N);
}