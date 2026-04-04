#pragma once

#include "resource.h"
#include <cstdint>
#include <string>
#include <vector>

namespace feather {

enum class TextureFormat : uint8_t {
	INVALID = 0,
	R8_UNORM,
	RG8_UNORM,
	RGBA8_UNORM,
	RGBA8_SRGB,
	R16_FLOAT,
	RG16_FLOAT,
	RGBA16_FLOAT,
	R32_FLOAT,
	RG32_FLOAT,
	RGBA32_FLOAT,
};

class Texture : public Resource {
	FCLASS(Texture, Resource);

protected:
	std::vector<uint8_t> _pixel_data;
	uint32_t _width = 0;
	uint32_t _height = 0;
	TextureFormat _format = TextureFormat::RGBA8_UNORM;
	bool _is_loaded = false;

	static void _bind_members();

public:
	Texture() = default;

	// File loading
	bool load_from_file();

	// Direct data setting (for procedural textures)
	void set_data(const std::string& path, const std::vector<uint8_t>& data, uint32_t width, uint32_t height,
			TextureFormat format);

	// Getters
	const std::vector<uint8_t>& get_pixel_data() const { return _pixel_data; }
	uint32_t get_width() const { return _width; }
	uint32_t get_height() const { return _height; }
	TextureFormat get_format() const { return _format; }
	bool is_loaded() const { return _is_loaded; }

	// Get bytes per pixel for format
	static uint32_t get_bytes_per_pixel(TextureFormat format);
};

} //namespace feather
