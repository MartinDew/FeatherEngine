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

template <class T, class TVariant>
constexpr bool is_variant_alternative = false;
template <class T, class... TAlternatives>
constexpr bool is_variant_alternative<T, std::variant<TAlternatives...>> = (std::is_same_v<T, TAlternatives> || ...);

// A type Variant can hold, or Variant itself, which a bound method takes or returns as is.
template <typename T>
concept VariantCompatible = std::is_same_v<T, Variant> || get_variant_type<T>() != VariantType::INVALID;

struct ClassInfo;

// An instance of a reflected value type: owned by the box and managed through the ClassInfo's lifecycle hooks, or a
// reference to one the caller keeps alive, which copies of the box share.
class FEATHER_REFLECTION_API ValueBox {
	const ClassInfo* _info = nullptr;
	void* _data = nullptr;
	bool _owned = true;

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

	// Refers to `data` without copying it; it must outlive every box that refers to it.
	static ValueBox ref(const ClassInfo& info, void* data);

	const ClassInfo* get_class_info() const noexcept { return _info; }
	bool is_reference() const noexcept { return !_owned; }
	void* get() noexcept { return _data; }
	const void* get() const noexcept { return _data; }
	bool is_class(StaticString name) const noexcept;
	// Same class, and equal by the type's operator== or else by every readable property.
	bool operator==(const ValueBox& other) const;
};

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

	// True for an object, a boxed value, and a type stored natively that has an FType (Vector3, Transform...).
	bool _is_reflectable() const;
	// A natively stored type is read through its FType directly, with no ClassInfo.
	bool _has_builtin_properties() const;
	[[nodiscard]] Variant _get_builtin(std::string_view key) const;
	void _set_builtin(std::string_view key, const Variant& value);

	// Visits the (class, storage) levels get/set reach: an object's hierarchy, or a value's single class.
	template <class TVisit>
	bool _walk(TVisit&& visit);

	[[nodiscard]] Variant _get_property(std::string_view key, bool enforce_public) const;
	void _set_property(std::string_view key, const Variant& value, bool enforce_public);

	// args[0] is left for the instance, which this fills. enforce_public denies non-Public methods; *_internal entry
	// points pass false.
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

	// An owned copy of a value type's instance; `src` null makes a default-constructed one. NIL when the class can't be
	// copied, or default-constructed for a null `src`.
	static Variant from_value(const ClassInfo& info, const void* src);
	static Variant default_value(const ClassInfo& info);
	// A VALUE that edits `data` in place, which must outlive the Variant and its copies. NIL for a non-value type.
	static Variant ref(const ClassInfo& info, void* data);
	template <class T>
		requires is_reflected_value_type<T>
	static Variant ref(T& value) {
		const ClassInfo* info = _find_class_info(FType<T>::name);
		fassert(info, "Value type isn't registered in ClassDB");
		return ref(*info, &value);
	}
	// The box of a VALUE variant; null otherwise.
	const ValueBox* get_value() const;

	// A natively stored math type (Vector*, Quaternion, Color, Matrix, Transform, Vertex) copied from raw memory of
	// that type. NIL for any other type.
	static Variant from_builtin(VariantType type, const void* src);
	// The storage of a natively stored math type, as from_builtin reads it; null for any other type.
	const void* builtin_data() const;

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
			else if constexpr (!std::is_pointer_v<T> && std::is_base_of_v<Reflected, std::remove_reference_t<T>>) {
				T* object = object_cast<T>(_object());
				if (!object) {
					return std::unexpected("Variant object is not of the requested type");
				}
				return *object;
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
	// return NIL, set is ignored otherwise).
	Variant get(std::string_view key) const;
	void set(std::string_view key, const Variant& value);
	// Object method call
	Variant call(std::string_view method_name);
	template <class... TArgs>
	Variant call(std::string_view method_name, TArgs&&... args);

	// Privileged access for trusted engine systems (e.g. editor inspector);
	// bypasses accessibility checks. Do not expose to scripts.
	Variant get_internal(std::string_view key) const;
	void set_internal(std::string_view key, const Variant& value);
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

	// The held instance itself rather than a copy, as a T& parameter binds to it. Only types stored as-is qualify: a
	// value type, a Reflected object, or a native alternative such as Vector3.
	template <class T>
		requires(!std::is_const_v<T>)
	explicit operator T&() {
		if constexpr (is_reflected_value_type<T>) {
			ValueBox* box = std::get_if<ValueBox>(&_data);
			fassert(box && box->is_class(FType<T>::name), "Variant value is not of the requested class");
			return *static_cast<T*>(box->get());
		}
		else if constexpr (std::is_base_of_v<Reflected, T>) {
			T* object = object_cast<T>(_object());
			fassert(object, "Variant object is not of the requested type");
			return *object;
		}
		else {
			static_assert(
					is_variant_alternative<T, InternalVariant>, "T isn't stored as-is, so it can't be referenced"
			);
			return std::get<T>(_data);
		}
	}
};

// Force std::span<Variant> to be instantiated in this partition. Otherwise GCC
// instantiates it first in :framework.callable, and importers then fail with
// "recursive lazy load ... failed to load pendings for 'feather::Variant'".
static_assert(sizeof(std::span<Variant>) > 0);

template <class... TArgs>
Variant Variant::call(std::string_view method_name, TArgs&&... args) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	Variant params[] = { Variant(), args... };
	return _internal_call(method_name, std::span<Variant>(params, sizeof...(args) + 1), true);
}

template <class... TArgs>
Variant Variant::call_internal(std::string_view method_name, TArgs&&... args) {
	fassert(_is_reflectable(), "Variant has no properties or methods");
	Variant params[] = { Variant(), args... };
	return _internal_call(method_name, std::span<Variant>(params, sizeof...(args) + 1), false);
}

} // namespace feather
