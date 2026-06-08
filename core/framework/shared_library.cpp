#include "shared_library.h"

#include "build/release-fbuild/_deps/sdl3-src/include/SDL3/SDL_loadso.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace feather {

SharedLibrary::SharedLibrary() : _handle(nullptr) {
}

SharedLibrary::~SharedLibrary() {
	unload();
}

bool SharedLibrary::load(const std::string& path) {
	unload();

	_handle = SDL_LoadObject(path.c_str());

	return _handle != nullptr;
}

void SharedLibrary::unload() {
	if (!_handle) {
		return;
	}

	SDL_UnloadObject(_handle);
	_handle = nullptr;
}

void* SharedLibrary::get_symbol(const std::string& name) const {
	if (!_handle)
		return nullptr;

	return SDL_LoadFunction(_handle, name.c_str());
}

bool SharedLibrary::is_loaded() const {
	return _handle != nullptr;
}

} // namespace feather
