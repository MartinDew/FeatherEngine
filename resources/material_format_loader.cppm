module;

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "material_format_loader.gen.h"
#endif

export module feather.resources:material_format_loader;

import std;
import feather.framework;
import :resource_format_loader;
import feather.reflection;

export namespace feather {

class FEATHER_RESOURCES_API MaterialFormatLoader : public ResourceFormatLoader {
	FCLASS();

protected:
	std::shared_ptr<Resource> instantiate(const Path& path) override;
	void load(std::shared_ptr<Resource> resource, const Path& path) override;

public:
	bool recognize_extension(const std::string& extension) const override;
};

} // namespace feather
