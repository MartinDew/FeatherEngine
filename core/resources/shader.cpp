#include "shader.h"

#include <framework/reflection_macros.h>

namespace feather {

void Shader::_bind_members() {}

INPLACE_REGISTER_BEGIN(Shader);
ClassDB::register_class<Shader>();
INPLACE_REGISTER_END(Shader);

} //namespace feather
