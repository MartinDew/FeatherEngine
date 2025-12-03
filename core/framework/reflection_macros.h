#pragma once

#define FCLASS(_name, _parent)                                                                                         \
	friend class ClassDB;                                                                                              \
	struct _class_type {};                                                                                             \
	using Type = _name;                                                                                                \
	using Super = _parent;                                                                                             \
	constexpr static StaticString get_class_name() { return #_name##_ss; }                                             \
	constexpr static StaticString get_parent_name() { return #_parent##_ss; }
