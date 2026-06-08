#pragma once

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

	void* get_symbol(const std::string& name) const;
	bool is_loaded() const;
};

} // namespace feather
