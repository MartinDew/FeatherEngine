#pragma once

#include "resource_format_loader.h"

namespace feather {

class TextureFormatLoader : public ResourceFormatLoader {
	FCLASS(TextureFormatLoader, ResourceFormatLoader);

protected:
	static void _bind_members();

	std::shared_ptr<Resource> load(const Path& path) override;

public:
	bool recognize_extension(const std::string& extension) const override;
};

} // namespace feather
