#pragma once

#include "resource.h"
#include <core/math/math_defs.h>

namespace feather {

class RenderingServer;
class Shader;
class Texture;

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

class PBRMaterial : public Material {
	FCLASS(PBRMaterial, Material);

protected:
	// Textures
	std::shared_ptr<Texture> _base_color_texture;
	std::shared_ptr<Texture> _metallic_roughness_texture;
	std::shared_ptr<Texture> _normal_texture;
	std::shared_ptr<Texture> _emissive_texture;

	// Material factors (multiply with texture samples)
	Color _base_color_factor = Color(1.0f, 1.0f, 1.0f, 1.0f);
	float _metallic_factor = 1.0f;
	float _roughness_factor = 1.0f;
	Color _emissive_factor = Color(0.0f, 0.0f, 0.0f, 1.0f);

	// Rendering options
	bool _alpha_blend = false;
	bool _double_sided = false;

	static void _bind_members();

public:
	PBRMaterial() = default;

	// Texture getters/setters
	void set_base_color_texture(std::shared_ptr<Texture> texture) { _base_color_texture = texture; }
	std::shared_ptr<Texture> get_base_color_texture() const { return _base_color_texture; }

	void set_metallic_roughness_texture(std::shared_ptr<Texture> texture) { _metallic_roughness_texture = texture; }
	std::shared_ptr<Texture> get_metallic_roughness_texture() const { return _metallic_roughness_texture; }

	void set_normal_texture(std::shared_ptr<Texture> texture) { _normal_texture = texture; }
	std::shared_ptr<Texture> get_normal_texture() const { return _normal_texture; }

	void set_emissive_texture(std::shared_ptr<Texture> texture) { _emissive_texture = texture; }
	std::shared_ptr<Texture> get_emissive_texture() const { return _emissive_texture; }

	// Factor getters/setters
	void set_base_color_factor(const Color& color) { _base_color_factor = color; }
	const Color& get_base_color_factor() const { return _base_color_factor; }

	void set_metallic_factor(float metallic) { _metallic_factor = metallic; }
	float get_metallic_factor() const { return _metallic_factor; }

	void set_roughness_factor(float roughness) { _roughness_factor = roughness; }
	float get_roughness_factor() const { return _roughness_factor; }

	void set_emissive_factor(const Color& color) { _emissive_factor = color; }
	const Color& get_emissive_factor() const { return _emissive_factor; }

	// Rendering options
	void set_alpha_blend(bool enabled) { _alpha_blend = enabled; }
	bool get_alpha_blend() const { return _alpha_blend; }

	void set_double_sided(bool enabled) { _double_sided = enabled; }
	bool get_double_sided() const { return _double_sided; }
};

} //namespace feather
