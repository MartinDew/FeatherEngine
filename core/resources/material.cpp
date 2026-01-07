#include "material.h"

namespace feather {

void Material::_bind_members() {}

INPLACE_REGISTER_BEGIN(Material);
ClassDB::register_class<Material>();
INPLACE_REGISTER_END(Material);

} //namespace feather