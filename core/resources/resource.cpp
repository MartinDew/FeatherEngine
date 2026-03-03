#include "resource.h"
#include <core/main/class_db.h>

namespace feather {

INPLACE_REGISTER_BEGIN(Resource);
ClassDB::register_abstract_class<Resource>();
INPLACE_REGISTER_END(Resource);

} // namespace feather