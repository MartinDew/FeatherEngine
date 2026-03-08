#include "resource.h"
#include <core/main/class_db.h>

namespace feather {

INPLACE_REGISTER_BEGIN(Resource);
ClassDB::register_abstract_class<Resource>();
INPLACE_REGISTER_END(Resource);

void Resource::_bind_members() {
	ClassDB::bind_method(&Resource::get_rid, "get_rid");
}

} // namespace feather