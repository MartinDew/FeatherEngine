#pragma once
#include "resource.h"

#include <filesystem>

namespace feather {

class RenderingServer;

class Shader : public Resource {
	FCLASS(Shader, Resource);
	friend RenderingServer;

	struct InternalData {
		enum class ShaderFormat : uint8_t {
			INVALID = 0,
			PATH = 1 << 0,
			SOURCE = 1 << 1
		};

		ShaderFormat shader_format = ShaderFormat::INVALID;
		std::variant<std::monostate, std::string, std::filesystem::path> _data = {};
	} _internal_data = {};

protected:
	static void _bind_members();

public:
	Shader() = default;
};

} //namespace feather