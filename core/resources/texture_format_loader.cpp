#include "texture_format_loader.h"
#include "texture.h"

namespace feather {

void TextureFormatLoader::_bind_members() {}

bool TextureFormatLoader::recognize_extension(const std::string& extension) const {
	return extension == "png" || extension == "jpg" || extension == "jpeg" || extension == "tga" || extension == "bmp";
}

std::shared_ptr<Resource> TextureFormatLoader::load(const Path& path) {
	std::shared_ptr<Texture> texture = std::make_shared<Texture>();
	texture->set_file_path(path.string());
	texture->load_from_file();
	return texture;
}

} // namespace feather
