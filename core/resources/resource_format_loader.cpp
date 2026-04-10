#include "resource_format_loader.h"
#include <core/main/class_db.h>

namespace feather {

void ResourceFormatLoader::_bind_members() {
	ClassDB::bind_method(&ResourceFormatLoader::recognize_extension, "recognize_extension");
}

} // namespace feather
