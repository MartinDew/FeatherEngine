#include "resource_format_loader.h"
#include <core/main/class_db.h>

namespace feather {

void ResourceFormatLoader::_bind_members() {
	ClassDB::bind_method(&ResourceFormatLoader::recognize_extension, "recognize_extension");
	// todo figure out how to make shared ptr interact with variant
	// ClassDB::bind_method(&ResourceFormatLoader::load, "load");
}

INPLACE_REGISTER_BEGIN(ResourceFormatLoader);
ClassDB::register_abstract_class<ResourceFormatLoader>();
INPLACE_REGISTER_END(ResourceFormatLoader);

} // namespace feather
