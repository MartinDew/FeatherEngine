#include "material.h"

#include "shader.h"
#include "texture.h"
#include <framework/variant.h>
#include <main/class_db.h>

namespace feather {

void Material::_bind_members() {
}

void PBRMaterial::_bind_members() {
	// Bind material factors
	ClassDB::bind_property(
			&PBRMaterial::_base_color_factor,
			"base_color_factor",
			VariantType::COLOR
	); // ClassDB::bind_property(&PBRMaterial::_base_color_factor, "base_color_factor",
	   // VariantType::OBJECT);
	ClassDB::bind_property(&PBRMaterial::_metallic_factor, "metallic_factor", VariantType::FLOAT);
	ClassDB::bind_property(&PBRMaterial::_roughness_factor, "roughness_factor", VariantType::FLOAT);
	ClassDB::bind_property(&PBRMaterial::_emissive_factor, "emissive_factor", VariantType::COLOR);

	// Bind rendering options
	ClassDB::bind_property(&PBRMaterial::_alpha_blend, "alpha_blend", VariantType::BOOL);
	ClassDB::bind_property(&PBRMaterial::_double_sided, "double_sided", VariantType::BOOL);
}

static std::shared_ptr<Shader> placeholder_mat_shader;

PlaceholderMaterial::PlaceholderMaterial() {
	if (!placeholder_mat_shader) {
		placeholder_mat_shader = std::make_shared<Shader>();
		// placeholder_mat_shader->set_shader_code(placeholder_mat);
	}

	_shader = placeholder_mat_shader;
}

} //namespace feather