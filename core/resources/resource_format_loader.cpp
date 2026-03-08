#include "resource_format_loader.h"
#include <core/main/class_db.h>

namespace feather {

void ResourceFormatLoader::_bind_members() {}

INPLACE_REGISTER_BEGIN(ResourceFormatLoader);
ClassDB::register_abstract_class<ResourceFormatLoader>();
INPLACE_REGISTER_END(ResourceFormatLoader);

} // namespace feather
