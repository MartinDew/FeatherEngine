#pragma once

#include "resource_format_loader.h"

namespace feather {

class ExtensionFormatLoader : public ResourceFormatLoader {
	FCLASS(ExtensionFormatLoader, ResourceFormatLoader);

protected:
	static void _bind_members();

	std::shared_ptr<Resource> instantiate(const Path& path) override;
	void load(std::shared_ptr<Resource> resource, const Path& path) override;
	bool requires_immediate_load() const override { return true; }

public:
	bool recognize_extension(const std::string& extension) const override;
};

} // namespace feather
