#include "material.h"

#include "shader.h"
#include <raw_resources/shaders/placeholder_mat.slang.h>

namespace feather {

void Material::_bind_members() {}

static std::shared_ptr<Shader> placeholder_mat_shader;

PlaceholderMaterial::PlaceholderMaterial() {
	if (!placeholder_mat_shader) {
		placeholder_mat_shader = std::make_shared<Shader>();
		// placeholder_mat_shader->set_shader_code(placeholder_mat);
	}

	_shader = placeholder_mat_shader;
}

INPLACE_REGISTER_BEGIN(Material);
ClassDB::register_class<Material>();
ClassDB::register_class<PlaceholderMaterial>();
ClassDB::register_class<ShaderMaterial>();
INPLACE_REGISTER_END(Material);

} //namespace feather