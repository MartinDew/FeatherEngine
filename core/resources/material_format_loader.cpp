#include "material_format_loader.h"
#include "material.h"

namespace feather {

void MaterialFormatLoader::_bind_members() {}

bool MaterialFormatLoader::recognize_extension(const std::string& extension) const {
	return extension == "mat";
}

std::shared_ptr<Resource> MaterialFormatLoader::load(const Path& path) {
	// Basic implementation since we lack a JSON/parsing library right now
	// Ideally, this parses the file at `path` and configures the material
	std::shared_ptr<PBRMaterial> material = std::make_shared<PBRMaterial>();
	return material;
}

} // namespace feather
