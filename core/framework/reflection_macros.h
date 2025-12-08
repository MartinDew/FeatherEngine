#pragma once

#include "main/class_db.inl"
#include "static_string.hpp"

template <class T>
concept has_is_type_method = requires { T::is_type(); };

template <has_is_type_method T>
static bool get_is_type(StaticString type_name) {
	T::is_of_type(type_name);
}

template <class T>
	requires(!has_is_type_method<T>)
static bool get_is_type(StaticString type_name) {
	return false;
}

struct NO_PARENT {};

#define FCLASS(_name, _parent)                                                                                         \
	friend class ClassDB;                                                                                              \
	struct _class_type {};                                                                                             \
	template <class T>                                                                                                 \
	friend void has_bind_method(const T& t);                                                                           \
                                                                                                                       \
protected:                                                                                                             \
	using Type = _name;                                                                                                \
	using Super = _parent;                                                                                             \
                                                                                                                       \
public:                                                                                                                \
	constexpr static StaticString get_class_static() { return #_name##_ss; }                                           \
	constexpr static StaticString get_parent_name() { return #_parent##_ss; }                                          \
	/*Maybe there's a better way to do casts than use vcalls? */                                                       \
	inline bool is_of_type(StaticString type_name) override {                                                          \
		return get_class_static() == type_name || Super::is_of_type(type_name);                                        \
	}                                                                                                                  \
	inline virtual StaticString get_class_name() { return get_class_static(); }                                        \
                                                                                                                       \
private:

// Utility to register a class to DB in place instead of in a register function context. Please only use within a CPP
// file
#define INPLACE_REGISTER_BEGIN(_class_name)                                                                            \
	struct register##_class_name {                                                                                     \
		register##_class_name() {
#define INPLACE_REGISTER_END(_class_name)                                                                              \
	}                                                                                                                  \
	}                                                                                                                  \
	register##_class_name##_instance
