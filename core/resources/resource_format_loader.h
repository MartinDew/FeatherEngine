#pragma once

#include "resource.h"
#include <core/framework/path.h>
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#include <string>

namespace feather {

class ResourceLoader;

class ResourceFormatLoader : public Reflected {
	FCLASS(ResourceFormatLoader, Reflected);
	friend ResourceLoader;

protected:
	static void _bind_members();

	virtual std::shared_ptr<Resource> load(const Path& path) = 0;

public:
	virtual bool recognize_extension(const std::string& extension) const = 0;
};

} // namespace feather
