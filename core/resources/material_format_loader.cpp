module;
#include <xstring>
#include <memory>

module feather.core;

namespace feather {

bool MaterialFormatLoader::recognize_extension(const std::string& extension) const {
	return extension == "mat";
}

std::shared_ptr<Resource> MaterialFormatLoader::instantiate(const Path& path) {
	auto material = std::make_shared<PBRMaterial>();
	material->set_path(path.string());
	return material;
}

void MaterialFormatLoader::load(std::shared_ptr<Resource> resource, const Path& path) {
	// Basic implementation since we lack a JSON/parsing library right now
	// Ideally, this parses the file at `path` and configures the material
}

} // namespace feather
