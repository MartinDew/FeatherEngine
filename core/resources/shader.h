#pragma once
#include "resource.h"

#include <filesystem>

namespace feather {

class RenderingServer;

class Shader : public Resource {
	FCLASS(Shader, Resource);
	friend RenderingServer;

	enum class ShaderSourceType : uint8_t {
		INVALID = 0,
		PATH,
		SOURCE,
	};

	ShaderSourceType _shader_source_type = ShaderSourceType::INVALID;
	std::string _shader_source = "";

protected:
	static void _bind_members();

public:
	Shader() = default;

	void set_shader_path(const std::string_view path);
	void set_shader_code(const std::string_view code);

	bool is_valid() const;
};

} //namespace feather