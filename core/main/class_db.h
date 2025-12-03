#pragma once

#include <memory>

namespace feather {

template <class T>
concept is_reflected_class_type = requires { typename T::_class_type; } && std::is_class_v<typename T::_class_type>;

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