#include "alloc.h"

#include <new>

// Not `inline` -- replaceable global allocation functions aren't allowed to be.
// Compiled once into every binary: the executable and, in shared mode, each module library.

void* operator new(std::size_t size) {
	if (void* ptr = feather_alloc(size)) {
		return ptr;
	}
	throw std::bad_alloc();
}

void* operator new[](std::size_t size) {
	return ::operator new(size);
}

void operator delete(void* ptr) noexcept {
	feather_free(ptr);
}

void operator delete[](void* ptr) noexcept {
	feather_free(ptr);
}

// Sized-delete overloads (C++14) -- the compiler may call these instead of
// the unsized ones above; must be routed the same way.
void operator delete(void* ptr, std::size_t) noexcept {
	feather_free(ptr);
}

void operator delete[](void* ptr, std::size_t) noexcept {
	feather_free(ptr);
}

// ---- nothrow new/delete -----------------------------------------------------

void* operator new(std::size_t size, const std::nothrow_t&) noexcept {
	return feather_alloc(size);
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept {
	return feather_alloc(size);
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept {
	feather_free(ptr);
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept {
	feather_free(ptr);
}

// ---- Aligned new/delete (C++17) --------------------------------------------

void* operator new(std::size_t size, std::align_val_t align) {
	if (void* ptr = feather_alloc_aligned(size, static_cast<std::size_t>(align))) {
		return ptr;
	}
	throw std::bad_alloc();
}

void* operator new[](std::size_t size, std::align_val_t align) {
	return ::operator new(size, align);
}

void operator delete(void* ptr, std::align_val_t align) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}

void operator delete[](void* ptr, std::align_val_t align) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}

void operator delete(void* ptr, std::size_t, std::align_val_t align) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}

void operator delete[](void* ptr, std::size_t, std::align_val_t align) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}

void* operator new(std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
	return feather_alloc_aligned(size, static_cast<std::size_t>(align));
}

void* operator new[](std::size_t size, std::align_val_t align, const std::nothrow_t&) noexcept {
	return feather_alloc_aligned(size, static_cast<std::size_t>(align));
}

void operator delete(void* ptr, std::align_val_t align, const std::nothrow_t&) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}

void operator delete[](void* ptr, std::align_val_t align, const std::nothrow_t&) noexcept {
	feather_free_aligned(ptr, static_cast<std::size_t>(align));
}
