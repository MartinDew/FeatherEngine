export module feather.reflection:variant;

import std.compat;
import feather.framework;
import :variant_array;
import :reflected;
import :reflection_utils;
import feather.math;

export namespace feather {

enum class VariantType : uint8_t {
	NIL = 0,
	BOOL,
	// Math
	INT,
	FLOAT,
	VECTOR3,
	VECTOR2,
	VERTEX,
	COLOR,
	VECTOR4,
	QUATERNION,
	MATRIX,
	TRANSFORM,
	// Misc
	RID,
	STRING,
	ARRAY,
	PATH,
	// Objects
	OBJECT,
	// A reflected value type owned by the Variant
	VALUE,
	// Invalid
	INVALID
};

// Operators Variant::evaluate applies. Unary ones (Negate, Positive, BitNegate, Not) ignore b.
enum class VariantOperator : uint8_t {
	// Comparison
	Equal,
	NotEqual,
	Less,
	LessEqual,
	Greater,
	GreaterEqual,
	// Arithmetic
	Add,
	Subtract,
	Multiply,
	Divide,
	Negate,
	Positive,
	Module,
	Power,
	// Bitwise (INT only)
	ShiftLeft,
	ShiftRight,
	BitAnd,
	BitOr,
	BitXor,
	BitNegate,
	// Logical: operands are booleanized
	And,
	Or,
	Xor,
	Not,
	// `a in b`: substring of a STRING, or element of an ARRAY
	In,
	MAX
};

#define VARIANT_TYPE_OPTION(class_name, variant_name)                                                                  \
	else if constexpr (std::is_same_v<T, class_name>) {                                                                \
		return VariantType::variant_name;                                                                              \
	}

// Helper to get VariantType enum from C++ type
template <class T>
consteval VariantType get_variant_type() {
	if constexpr (std::is_same_v<T, bool>) {
		return VariantType::BOOL;
	}
	else if constexpr ((std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_enum_v<T>) {
		return VariantType::INT;
	}
	else if constexpr (std::is_floating_point_v<T>) {
		return VariantType::FLOAT;
	}
	else if constexpr (std::is_same_v<Vector3, T>) {
		return VariantType::VECTOR3;
	}
	else if constexpr (std::is_same_v<Vector2, T>) {
		return VariantType::VECTOR2;
	}
	else if constexpr (std::is_same_v<Color, T>) {
		return VariantType::COLOR;
	}
	else if constexpr (std::is_same_v<Vertex, T>) {
		return VariantType::VERTEX;
	}
	else if constexpr (std::is_same_v<Vector4, T>) {
		return VariantType::VECTOR4;
	}
	else if constexpr (std::is_same_v<Quaternion, T>) {
		return VariantType::QUATERNION;
	}
	else if constexpr (std::is_same_v<Matrix, T>) {
		return VariantType::MATRIX;
	}
	else if constexpr (std::is_same_v<Transform, T>) {
		return VariantType::TRANSFORM;
	}
	else if constexpr (std::is_same_v<T, std::string>) {
		return VariantType::STRING;
	}
	else if constexpr (std::is_same_v<T, VariantArray> || std::is_array_v<T> || is_contiguous_container<T>) {
		return VariantType::ARRAY;
	}
	VARIANT_TYPE_OPTION(RID, RID)
	VARIANT_TYPE_OPTION(Path, PATH)
	// object
	else if constexpr (
			(std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_pointer_t<T>>) ||
			std::is_base_of_v<Reflected, T>
	) {
		return VariantType::OBJECT;
	}
	else if constexpr (is_reflected_value_type<T>) {
		return VariantType::VALUE;
	}
	// Void case
	else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_void_v<T>) {
		return VariantType::NIL;
	}
	// Default case
	else {
		return VariantType::INVALID;
	}
}

class Variant;

// A type Variant can hold, or Variant itself, which a bound method takes or returns as is.
template <typename T>
concept VariantCompatible = std::is_same_v<T, Variant> || get_variant_type<T>() != VariantType::INVALID;

struct ClassInfo;

} // namespace feather

// Not exported: Variant's storage for a reflected value type, which users reach through get_instance().
namespace feather {

// An instance of a reflected value type, owned by its box and managed through the ClassInfo's lifecycle hooks.
class FEATHER_REFLECTION_API ValueBox {
	const ClassInfo* _info = nullptr;
	void* _data = nullptr;

	void _destroy() noexcept;

public:
	ValueBox() = default;
	// Copies `src`, which needs a copyable type; a null `src` is a default-constructed instance of a creatable type.
	ValueBox(const ClassInfo& info, const void* src);
	ValueBox(const ValueBox& other);
	ValueBox(ValueBox&& other) noexcept;
	ValueBox& operator=(const ValueBox& other);
	ValueBox& operator=(ValueBox&& other) noexcept;
	~ValueBox();

	const ClassInfo* get_class_info() const noexcept { return _info; }
	void* get() noexcept { return _data; }
	const void* get() const noexcept { return _data; }
	bool is_class(StaticString name) const noexcept;
	// Same class, and equal by the type's operator== or else by every readable property.
	bool operator==(const ValueBox& other) const;
};

} // namespace feather

export namespace feather {

class FEATHER_REFLECTION_API Variant {
	using InternalVariant = std::variant<
			std::monostate,
			bool,
			int64_t,
			real_t,
			Vector2,
			Vector3,
			Vertex,
			Color,
			Vector4,
			Quaternion,
			Matrix,
			Transform,
			std::string,
			Path,
			VariantArray,
			RID,
			Reflected*,
			ValueBox>;

	InternalVariant _data;
	VariantType _type;

	// The object of an OBJECT variant; null otherwise.
	Reflected* _object() const;

	static const ClassInfo* _find_class_info(StaticString name);

	inline bool _is_reflectable() const { return get_instance().info != nullptr; }

	// Visits each (class, instance adjusted to it) level that get/set/call search, most derived first.
	template <class TVisit>
	bool _walk(TVisit&& visit) const;

	[[nodiscard]] Variant _get_property(std::string_view key, bool enforce_public) const;
	bool _set_property(std::string_view key, const Variant& value, bool enforce_public);

	// enforce_public denies non-Public members; the *_internal entry points pass false.
	[[nodiscard]] Variant _internal_call(std::string_view method_name, std::span<Variant> args, bool enforce_public);

public:
	// Default constructor - NIL
	inline Variant() : _data(std::monostate {}), _type(VariantType::NIL) {}

	// A reflected object is referenced, never copied in.
	template <class T>
		requires(!std::is_const_v<T> && std::is_base_of_v<Reflected, T>)
	Variant(T& object) : _data { static_cast<Reflected*>(&object) }
					   , _type { VariantType::OBJECT } {}

	// String literal constructor
	inline Variant(const char* str) : _data(std::string(str)), _type(VariantType::STRING) {}

	// Generic constructor with concept constraint
	template <VariantCompatible T>
		requires(!std::is_base_of_v<Reflected, T> && !std::is_same_v<T, Variant>)
	Variant(T value) {
		constexpr VariantType type = get_variant_type<T>();
		_type = type;

		if constexpr (type == VariantType::BOOL) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::INT) {
			_data = static_cast<int64_t>(value);
		}
		else if constexpr (type == VariantType::FLOAT) {
			_data = static_cast<real_t>(value);
		}
		else if constexpr (type == VariantType::VECTOR3) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::STRING) {
			_data = std::move(value);
		}
		else if constexpr (type == VariantType::ARRAY) {
			if constexpr (std::is_same_v<T, VariantArray>)
				_data = std::move(value);
			else {
				_data = VariantArray { value.begin(), value.end() };
			}
		}
		else if constexpr (type == VariantType::OBJECT) {
			_data = static_cast<Reflected*>(value);
		}
		else if constexpr (type == VariantType::VALUE) {
			const ClassInfo* info = _find_class_info(FType<T>::name);
			fassert(info, "Value type isn't registered in ClassDB");
			_data = ValueBox(*info, &value);
		}
		else if constexpr (type == VariantType::NIL) {
			_data = std::monostate {};
		}
		else if constexpr (type == VariantType::INVALID) {
			static_assert(false, "Variant type is unrecognized");
		}
		// default case if assignment is 1:1
		else {
			_data = std::move(value);
		}
	}

	struct Instance {
		void* data = nullptr;
		const ClassInfo* info = nullptr;
		// Keeps `info` alive when it was built for this call (a built-in's) rather than owned by ClassDB.
		std::shared_ptr<const ClassInfo> owned = nullptr;
	};
	// The held instance and its class, the same way for an object, a boxed value and a built-in (Vector3...); both
	// null when the held type has no members. An object's data is its Reflected*.
	Instance get_instance() const;

	// A copy of a value type's or built-in's instance; `src` null makes a default-constructed one. NIL for an object
	// class, or one that can't be copied (default-constructed for a null `src`).
	static Variant from_value(const ClassInfo& info, const void* src);
	static Variant default_value(const ClassInfo& info);

	// Copy and move constructors
	Variant(const Variant& other) = default;
	Variant(Variant&& other) noexcept = default;

	// Assignment operators
	Variant& operator=(const Variant& other) = default;
	Variant& operator=(Variant&& other) noexcept = default;

	// Type checking
	inline VariantType get_type() const { return _type; }
	inline bool is_type(VariantType type) const { return type == _type; }
	inline bool is_nil() const { return _type == VariantType::NIL; }

	// Type conversion with std::expected
	template <VariantCompatible T>
	std::expected<T, std::string> as() const {
		if constexpr (std::is_same_v<T, Variant>) {
			return *this;
		}
		else {
			if (get_variant_type<T>() != _type)
				return std::unexpected("Variant type does not match requested type");

			if constexpr (std::is_same_v<T, bool>) {
				return std::get<bool>(_data);
			}
			else if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
				return static_cast<T>(std::get<int64_t>(_data));
			}
			else if constexpr (std::is_floating_point_v<T>) {
				return static_cast<T>(std::get<real_t>(_data));
			}
			else if constexpr (std::is_enum_v<T>) {
				return static_cast<T>(std::get<int64_t>(_data));
			}
			else if constexpr (is_reflected_value_type<T>) {
				const ValueBox& box = std::get<ValueBox>(_data);
				if (!box.is_class(FType<T>::name)) {
					return std::unexpected("Variant value is not of the requested class");
				}
				return *static_cast<const T*>(box.get());
			}
			else if constexpr (std::is_base_of_v<Reflected, T>) {
				static_assert(false, "An object is held by reference: read it as T*, not as a copy");
			}
			else if constexpr (std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_pointer_t<T>>) {
				Reflected* reflected = _object();
				if (!reflected) {
					return static_cast<T>(nullptr);
				}
				T object = object_cast<std::remove_pointer_t<T>>(reflected);
				if (!object) {
					return std::unexpected("Variant object is not of the requested type");
				}
				return object;
			}
			else
				return std::get<T>(_data);
		}
	}

	// Equality operators
	bool operator==(const Variant& other) const;

	inline bool operator!=(const Variant& other) const { return !(*this == other); }

	// String conversion for debugging
	std::string to_string() const;
	static std::string_view get_type_name(VariantType type);

	// Evaluates `a op b`. Equal/NotEqual on unrelated types give false/true instead of an error; operator== above
	// stays strict (same type and value).
	static std::expected<Variant, std::string> evaluate(VariantOperator op, const Variant& a, const Variant& b = {});
	// Result type of `a op b` for these operand types, or INVALID when the combination isn't supported.
	static VariantType get_operator_return_type(VariantOperator op, VariantType a, VariantType b = VariantType::NIL);
	static std::string_view get_operator_name(VariantOperator op);

	// False for NIL, false, 0, 0.0, an empty string, path or array, a null object, and a math value equal to its
	// default (zero vector, identity quaternion...).
	bool booleanize() const;

	// C++ conveniences over evaluate(); unsupported operands fail an fassert. Hidden friends, so `vector < vector`
	// never converts to Variant; the T overloads keep `variant + 1` unambiguous.
	static Variant _evaluate_or_assert(VariantOperator op, const Variant& a, const Variant& b);

#define FEATHER_VARIANT_OPERATOR(sym, op, ret, unwrap)                                                                 \
	friend ret operator sym(const Variant& a, const Variant& b) {                                                      \
		return _evaluate_or_assert(VariantOperator::op, a, b) unwrap;                                                  \
	}                                                                                                                  \
	template <VariantCompatible T>                                                                                     \
	friend ret operator sym(const Variant& a, const T& b) {                                                            \
		return _evaluate_or_assert(VariantOperator::op, a, Variant(b)) unwrap;                                         \
	}                                                                                                                  \
	template <VariantCompatible T>                                                                                     \
	friend ret operator sym(const T& a, const Variant& b) {                                                            \
		return _evaluate_or_assert(VariantOperator::op, Variant(a), b) unwrap;                                         \
	}

	FEATHER_VARIANT_OPERATOR(+, Add, Variant, )
	FEATHER_VARIANT_OPERATOR(-, Subtract, Variant, )
	FEATHER_VARIANT_OPERATOR(*, Multiply, Variant, )
	FEATHER_VARIANT_OPERATOR(/, Divide, Variant, )
	FEATHER_VARIANT_OPERATOR(%, Module, Variant, )
	FEATHER_VARIANT_OPERATOR(<, Less, bool, .as<bool>().value())
	FEATHER_VARIANT_OPERATOR(<=, LessEqual, bool, .as<bool>().value())
	FEATHER_VARIANT_OPERATOR(>, Greater, bool, .as<bool>().value())
	FEATHER_VARIANT_OPERATOR(>=, GreaterEqual, bool, .as<bool>().value())

#undef FEATHER_VARIANT_OPERATOR

	friend Variant operator-(const Variant& a) { return _evaluate_or_assert(VariantOperator::Negate, a, {}); }
	friend Variant operator+(const Variant& a) { return _evaluate_or_assert(VariantOperator::Positive, a, {}); }

	// Direct access to internal variant (for advanced usage)
	inline const InternalVariant& internal() const { return _data; }

	// Object property access
	std::string get_name() const;

	// Script-facing: only AccessLevel::Public members are reachable (get/call
	// return NIL, set returns false otherwise).
	Variant get(std::string_view key) const;
	// False when the property is missing, unreachable, read-only or of another type; the value is then unchanged.
	bool set(std::string_view key, const Variant& value);
	// Object method call
	Variant call(std::string_view method_name);
	template <class... TArgs>
	Variant call(std::string_view method_name, TArgs&&... args);

	// Privileged access for trusted engine systems (e.g. editor inspector);
	// bypasses accessibility checks. Do not expose to scripts.
	Variant get_internal(std::string_view key) const;
	bool set_internal(std::string_view key, const Variant& value);
	Variant call_internal(std::string_view method_name);
	template <class... TArgs>
	Variant call_internal(std::string_view method_name, TArgs&&... args);

	// implicit conversion
	// In variant.h, inside the Variant class:

	// Implicit conversion operators
	inline explicit operator bool() const { return as<bool>().value(); }
	inline operator int() const { return as<int>().value(); }
	inline operator int64_t() const { return as<int64_t>().value(); }
	inline operator real_t() const { return as<real_t>().value(); }
	inline operator Vector2() const { return as<Vector2>().value(); }
	inline operator Vector3() const { return as<Vector3>().value(); }
	inline operator Color() const { return as<Color>().value(); }
	inline operator Vertex() const { return as<Vertex>().value(); }
	inline operator Vector4() const { return as<Vector4>().value(); }
	inline operator Quaternion() const { return as<Quaternion>().value(); }
	inline operator Matrix() const { return as<Matrix>().value(); }
	inline operator Transform() const { return as<Transform>().value(); }
	inline operator std::string() const { return as<std::string>().value(); }
	inline operator Path() const { return as<Path>().value(); }
	inline operator RID() const { return as<RID>().value(); }
	inline operator VariantArray() const { return as<VariantArray>().value(); }
	// Null when the Variant holds no object.
	inline operator Reflected*() const { return _object(); }
};

// Force std::span<Variant> to be instantiated in this partition. Otherwise GCC
// instantiates it first in :framework.callable, and importers then fail with
// "recursive lazy load ... failed to load pendings for 'feather::Variant'".
static_assert(sizeof(std::span<Variant>) > 0);

template <class... TArgs>
Variant Variant::call(std::string_view method_name, TArgs&&... args) {
	std::array<Variant, sizeof...(TArgs)> params { Variant(std::forward<TArgs>(args))... };
	return _internal_call(method_name, params, true);
}

template <class... TArgs>
Variant Variant::call_internal(std::string_view method_name, TArgs&&... args) {
	std::array<Variant, sizeof...(TArgs)> params { Variant(std::forward<TArgs>(args))... };
	return _internal_call(method_name, params, false);
}

} // namespace feather

// Not exported: the marshalling behind Callable and ClassDB's method thunks, which instantiate it in user code.
namespace feather {

// How a bound function's parameter of type A is read from its Variant. An object parameter is the Variant's object,
// copied only when A is by value; any other parameter, T& included, gets a converted copy.
template <class A>
struct VariantParam {
	using value_type = std::remove_cvref_t<A>;
	static constexpr bool refers_to_object = std::is_base_of_v<Reflected, value_type>;
	using storage = std::conditional_t<refers_to_object, value_type*, std::decay_t<A>>;

	static storage read(const Variant& param) {
		auto value = param.as<storage>();
		fassert(value.has_value(), "Argument Variant doesn't match the parameter type");
		if constexpr (refers_to_object) {
			fassert(*value, "Null object passed for an object reference parameter");
		}
		return *value;
	}

	static A pass(storage& value) {
		if constexpr (refers_to_object) {
			return *value;
		}
		else if constexpr (std::is_lvalue_reference_v<A>) {
			return value;
		}
		else {
			return std::move(value);
		}
	}
};

template <class R, class... A>
concept VariantBindable = VariantCompatible<std::decay_t<R>> && (VariantCompatible<std::decay_t<A>> && ...);

// Calls `function` with `params` converted to A..., and wraps its result.
template <class R, class... A, class F>
Variant invoke_with_variants(F&& function, std::span<Variant> params) {
	fassert(params.size() == sizeof...(A),
			std::format("Expected {} arguments and got {}", sizeof...(A), params.size()));
	return [&]<std::size_t... I>(std::index_sequence<I...>) -> Variant {
		std::tuple<typename VariantParam<A>::storage...> storage { VariantParam<A>::read(params[I])... };
		if constexpr (std::is_void_v<R>) {
			std::forward<F>(function)(VariantParam<A>::pass(std::get<I>(storage))...);
			return {};
		}
		else {
			return Variant(std::forward<F>(function)(VariantParam<A>::pass(std::get<I>(storage))...));
		}
	}(std::index_sequence_for<A...> {});
}

} // namespace feather
