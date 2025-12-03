#include "class_db.h"
#include <memory>

namespace feather {

std::unique_ptr<ClassDB> ClassDB::_instance = nullptr;

ClassDB& ClassDB::get() {
	if (!_instance) {
		_instance.reset(new ClassDB());
	}
	return *_instance;
}

} //namespace feather