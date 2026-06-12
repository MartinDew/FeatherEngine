#pragma once

#include "callable.h"

#include <memory>
#include <string>

struct SDL_SharedObject;

namespace feather {

class SharedLibrary {
	SDL_SharedObject* _handle;

public:
	SharedLibrary();
	~SharedLibrary();

	bool load(const std::string& path);
	void unload();

	[[nodiscard]] Callable get_symbol(const std::string& name) const;
	[[nodiscard]] bool is_loaded() const;
};

} // namespace feather
