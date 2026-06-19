#include "simulation.h"

namespace feather {

void Simulation::_bind_members() {
	ClassDB::bind_method(&Type::init, "init");
	ClassDB::bind_method(&Type::update, "update");
	ClassDB::bind_method(&Type::fixed_update, "fixed_update");
}

} //namespace feather