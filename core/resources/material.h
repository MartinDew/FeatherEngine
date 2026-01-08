#pragma once

#include "resource.h"

namespace feather {

class RenderingServer;
class Shader;

class Material : public Resource {
	FCLASS(Material, Resource);
	friend RenderingServer;

protected:
	std::shared_ptr<Shader> _shader;

	// Todo : shader params
	static void _bind_members();
};

class PlaceholderMaterial : public Material {
	FCLASS(PlaceholderMaterial, Material)
public:
	PlaceholderMaterial();
};

class ShaderMaterial : public Material {
	FCLASS(ShaderMaterial, Material);

public:
	void set_shader(Shader shader);
};

} //namespace feather
