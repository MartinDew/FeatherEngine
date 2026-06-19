#pragma once

#include "resource_format_loader.h"

namespace feather {

class MaterialFormatLoader : public ResourceFormatLoader {
	FCLASS(MaterialFormatLoader, ResourceFormatLoader);

protected:
	static void _bind_members();

	std::shared_ptr<Resource> instantiate(const Path& path) override;
	void load(std::shared_ptr<Resource> resource, const Path& path) override;

public:
	bool recognize_extension(const std::string& extension) const override;
};

} // namespace feather
