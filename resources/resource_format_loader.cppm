module;

#include <framework/reflection_macros.h>

#ifndef FEATHER_REFLECTION_PARSER
#include "resource_format_loader.gen.h"
#endif

export module feather.resources:resource_format_loader;

import std;
import feather.framework;
import :resource;
import feather.reflection;

export namespace feather {

class ResourceLoader;

class FEATHER_RESOURCES_API ResourceFormatLoader : public Reflected {
	FCLASS();
	friend ResourceLoader;

protected:
	// Create a resource instance with path set but data not loaded
	virtual std::shared_ptr<Resource> instantiate(const Path& path) = 0;

	// Fill data into an existing resource instance (called after instantiate, or for reload)
	virtual void load(std::shared_ptr<Resource> resource, const Path& path) = 0;

	// If true, index_project calls load() immediately after instantiate
	virtual bool requires_immediate_load() const { return false; }

	ResourceFormatLoader() = default;

public:
	[[method]]
	virtual bool recognize_extension(const std::string& extension) const = 0;
};

} // namespace feather
