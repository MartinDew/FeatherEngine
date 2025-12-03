#pragma once

#include <memory>

namespace feather {

class ClassDB {
	static std::unique_ptr<ClassDB> _instance;
	ClassDB() = default;

public:
	ClassDB& get();

	template <class T>
	static void register_class();
};

} //namespace feather

#include "class_db.inl"