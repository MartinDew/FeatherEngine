module;

#include <framework/export_defs.h>

#include <framework/reflection_macros.h>
#ifndef FEATHER_REFLECTION_PARSER
#include "texture_format_loader.gen.h"
#endif

export module feather.core:resources.texture_format_loader;

import :resources.resource_format_loader;
import :main.class_db;

export namespace feather {

class FEATHER_API TextureFormatLoader : public ResourceFormatLoader {
	FCLASS();

protected:
	std::shared_ptr<Resource> instantiate(const Path& path) override;
	void load(std::shared_ptr<Resource> resource, const Path& path) override;

public:
	bool recognize_extension(const std::string& extension) const override;
};

} // namespace feather
