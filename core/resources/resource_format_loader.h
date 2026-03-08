#pragma once

#include "resource.h"
#include <framework/reflected.h>
#include <framework/reflection_macros.h>

#include <string>

namespace feather {

class ResourceFormatLoader : public Reflected {
	FCLASS(ResourceFormatLoader, Reflected);

protected:
	static void _bind_members();

public:
	virtual bool recognize_extension(const std::string& extension) const = 0;
	virtual std::shared_ptr<Resource> load(const std::string& path) = 0;
};

} // namespace feather
