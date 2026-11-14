module;

export module feather.reflection:reflected;

import std;
import :ftype;
import :reflection_utils;
import feather.framework;

export namespace feather {
// base class for reflected objects
class FEATHER_REFLECTION_API Reflected {
	friend class ClassDB;
	struct _class_type {};

protected:
	using Type = Reflected;

public:
	using _feather_self = Reflected;

	virtual ~Reflected() = default;

	constexpr static StaticString get_class_static() { return "Reflected"_ss; }
	constexpr static StaticString get_parent_name() { return ""_ss; }
	inline virtual bool is_of_type(StaticString type_name) const { return get_class_static() == type_name; }
	virtual StaticString get_class_name() = 0;
	template <is_reflected T>
	bool is_of_type() const {
		return is_of_type(T::get_class_static());
	}
};
} //namespace feather

namespace feather {

template <>
struct FType<Reflected> {
	using type = Reflected;
	using parent = void;
	static constexpr StaticString name = "Reflected"_ss;
	static constexpr StaticString parent_name = ""_ss;
	static constexpr auto properties = std::tuple {};
	static constexpr auto methods = std::tuple {};
};

} // namespace feather
