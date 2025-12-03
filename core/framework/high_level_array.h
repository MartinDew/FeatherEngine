#pragma once
#include "cow_vector.h"
#include "variant.h"

namespace feather {

class HighLevelArray : public CowVector<Variant> {
public:
};

} //namespace feather