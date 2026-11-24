export module feather.reflection:enum_info;

import std.compat;
import feather.framework;

export namespace feather {

struct Enumerator {
	StaticString name;
	int64_t value = 0;
};

// Compile-time description of a reflected enum, specialized by codegen: `name` and `values` (a std::array of
// Enumerator). Left undefined, so registering an enum without a description is a compile error.
template <class E>
struct FEnum;

// Runtime description of an enum, viewing its FEnum's constexpr values.
struct FEATHER_REFLECTION_API EnumInfo {
	StaticString name = ""_ss;
	size_t underlying_size = 0;
	std::span<const Enumerator> values;

	template <class E>
		requires std::is_enum_v<E>
	static constexpr EnumInfo create_from_type() {
		return { .name = FEnum<E>::name, .underlying_size = sizeof(E), .values = FEnum<E>::values };
	}

	// Null when no enumerator has this value (e.g. a flag combination).
	const Enumerator* find(int64_t value) const {
		auto it = std::ranges::find(values, value, &Enumerator::value);
		return it == values.end() ? nullptr : &*it;
	}

	const Enumerator* find(std::string_view enumerator_name) const {
		auto it = std::ranges::find_if(values, [&](const Enumerator& e) { return e.name.str() == enumerator_name; });
		return it == values.end() ? nullptr : &*it;
	}

	std::optional<std::string_view> name_of(int64_t value) const {
		const Enumerator* e = find(value);
		return e ? std::optional { e->name.str() } : std::nullopt;
	}

	std::optional<int64_t> value_of(std::string_view enumerator_name) const {
		const Enumerator* e = find(enumerator_name);
		return e ? std::optional { e->value } : std::nullopt;
	}
};

template <class E>
inline constexpr EnumInfo enum_info_v = EnumInfo::create_from_type<E>();

} // namespace feather
