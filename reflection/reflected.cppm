module;

export module feather.reflection:reflected;

import :reflection_utils;
import feather.framework;

export namespace feather {
// base class for reflected objects
class FEATHER_REFLECTION_API Reflected {
	struct _class_type {};

protected:
	using Type = Reflected;

public:
	// Public because ClassDB (core, above this module) can't be befriended from here.
	static void _bind_members();

	virtual ~Reflected() = default;

	constexpr static StaticString get_class_static() { return "Reflected"_ss; }
	constexpr static StaticString get_parent_name() { return ""_ss; }
	inline virtual bool is_of_type(StaticString type_name) const { return get_class_static() == type_name; }
	virtual StaticString get_class_name() = 0;
	template <is_reflected_class_type T>
	bool is_of_type() const {
		return is_of_type(T::get_class_static());
	}
};
} //namespace feather
