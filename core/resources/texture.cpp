#include "texture.h"
#include <core/framework/variant.h>
#include <main/class_db.h>

// TODO: Add image loading
// For now, basic implementation without actual file loading

namespace feather {

void Texture::_bind_members() {
	ClassDB::bind_property(&Texture::_file_path, "file_path", VariantType::STRING);
	ClassDB::bind_property(&Texture::_width, "width", VariantType::INT);
	ClassDB::bind_property(&Texture::_height, "height", VariantType::INT);
}

void Texture::set_file_path(const std::string_view path) {
	_file_path = path;
	_is_loaded = false;
}

bool Texture::load_from_file() {
	if (_file_path.empty()) {
		return false;
	}

	// TODO: Implement actual image loading using stb_image or DirectXTex
	// For now, return false to indicate not loaded
	// When implemented, this should:
	// 1. Load image from _file_path
	// 2. Decode pixel data
	// 3. Store in _pixel_data
	// 4. Set _width, _height, _format
	// 5. Set _is_loaded = true

	_is_loaded = false;
	return false;
}

void Texture::set_data(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, TextureFormat format) {
	_pixel_data = data;
	_width = width;
	_height = height;
	_format = format;
	_is_loaded = true;
	_file_path.clear();
}

uint32_t Texture::get_bytes_per_pixel(TextureFormat format) {
	switch (format) {
	case TextureFormat::R8_UNORM:
		return 1;
	case TextureFormat::RG8_UNORM:
		return 2;
	case TextureFormat::RGBA8_UNORM:
	case TextureFormat::RGBA8_SRGB:
		return 4;
	case TextureFormat::R16_FLOAT:
		return 2;
	case TextureFormat::RG16_FLOAT:
		return 4;
	case TextureFormat::RGBA16_FLOAT:
		return 8;
	case TextureFormat::R32_FLOAT:
		return 4;
	case TextureFormat::RG32_FLOAT:
		return 8;
	case TextureFormat::RGBA32_FLOAT:
		return 16;
	default:
		return 0;
	}
}

INPLACE_REGISTER_BEGIN(Texture);
ClassDB::register_class<Texture>();
INPLACE_REGISTER_END(Texture);

} //namespace feather
