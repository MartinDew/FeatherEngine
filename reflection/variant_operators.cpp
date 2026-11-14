module feather.reflection;

import std.compat;

namespace feather {

namespace {

// C++ type held for each VariantType. The enum and InternalVariant orders differ, so this can't be index-based.
template <VariantType>
struct variant_storage;

#define FEATHER_VARIANT_STORAGE(variant_type, cpp_type)                                                                \
	template <>                                                                                                        \
	struct variant_storage<VariantType::variant_type> {                                                                \
		using type = cpp_type;                                                                                         \
	};

FEATHER_VARIANT_STORAGE(NIL, std::monostate)
FEATHER_VARIANT_STORAGE(BOOL, bool)
FEATHER_VARIANT_STORAGE(INT, int)
FEATHER_VARIANT_STORAGE(FLOAT, real_t)
FEATHER_VARIANT_STORAGE(VECTOR3, Vector3)
FEATHER_VARIANT_STORAGE(VECTOR2, Vector2)
FEATHER_VARIANT_STORAGE(VERTEX, Vertex)
FEATHER_VARIANT_STORAGE(COLOR, Color)
FEATHER_VARIANT_STORAGE(VECTOR4, Vector4)
FEATHER_VARIANT_STORAGE(QUATERNION, Quaternion)
FEATHER_VARIANT_STORAGE(MATRIX, Matrix)
FEATHER_VARIANT_STORAGE(TRANSFORM, Transform)
FEATHER_VARIANT_STORAGE(RID, RID)
FEATHER_VARIANT_STORAGE(STRING, std::string)
FEATHER_VARIANT_STORAGE(ARRAY, VariantArray)
FEATHER_VARIANT_STORAGE(PATH, Path)
FEATHER_VARIANT_STORAGE(OBJECT, ObjectRef)

#undef FEATHER_VARIANT_STORAGE

template <VariantType T>
using storage_t = typename variant_storage<T>::type;

constexpr size_t operator_count = static_cast<size_t>(VariantOperator::MAX);
constexpr size_t type_count = static_cast<size_t>(VariantType::INVALID);

template <class T>
concept number = std::is_same_v<T, int> || std::is_same_v<T, real_t>;

template <class T>
concept math_value = std::is_same_v<T, Vector2> || std::is_same_v<T, Vector3> || std::is_same_v<T, Vector4> ||
		std::is_same_v<T, Color> || std::is_same_v<T, Quaternion> || std::is_same_v<T, Matrix> ||
		std::is_same_v<T, Transform>;

// BOOL, NIL, OBJECT, RID and PATH never take part in arithmetic, even where C++ would allow it (bool + int).
template <class T>
concept arithmetic = number<T> || math_value<T>;

// An INT next to anything but another INT is used as a real_t, so 1 + 0.5 and Vector3 * 2 work.
template <class T, class Other>
constexpr auto promote(const T& value) {
	if constexpr (std::is_same_v<T, int> && !std::is_same_v<Other, int>) {
		return static_cast<real_t>(value);
	}
	else {
		return value;
	}
}

template <class A, class B>
using promoted_t = decltype(promote<A, B>(std::declval<const A&>()));

// Marks an unsupported operand combination.
struct Unsupported {};

constexpr int int_pow(int base, int exponent) {
	int result = 1;
	while (exponent > 0) {
		if (exponent & 1) {
			result *= base;
		}
		base *= base;
		exponent >>= 1;
	}
	return result;
}

// The value of `a op b`, or Unsupported. Every branch is guarded, so the deduced return type alone tells whether the
// combination is supported. Value errors (division by zero...) are checked separately, in check_operands.
template <VariantOperator Op, class A, class B>
auto apply(const A& a, const B& b, const Variant& va, const Variant& vb) {
	using enum VariantOperator;
	using PA = promoted_t<A, B>;
	using PB = promoted_t<B, A>;
	constexpr bool unary = Op == Negate || Op == Positive || Op == BitNegate || Op == Not;
	constexpr bool nil_b = std::is_same_v<B, std::monostate>;
	[[maybe_unused]] auto pa = [&] { return promote<A, B>(a); };
	[[maybe_unused]] auto pb = [&] { return promote<B, A>(b); };

	if constexpr (unary && !nil_b) {
		return Unsupported {};
	}
	// Comparison: numbers compare by value, anything else only against its own type.
	else if constexpr (Op >= Equal && Op <= GreaterEqual) {
		if constexpr (number<A> && number<B>) {
			if constexpr (Op == Equal) return pa() == pb();
			else if constexpr (Op == NotEqual) return pa() != pb();
			else if constexpr (Op == Less) return pa() < pb();
			else if constexpr (Op == LessEqual) return pa() <= pb();
			else if constexpr (Op == Greater) return pa() > pb();
			else return pa() >= pb();
		}
		else if constexpr (!std::is_same_v<A, B>) {
			return Unsupported {};
		}
		else if constexpr ((Op == Equal || Op == NotEqual) && requires { a == b; }) {
			return Op == Equal ? static_cast<bool>(a == b) : !static_cast<bool>(a == b);
		}
		else if constexpr (
				Op != Equal && Op != NotEqual && !std::is_same_v<A, std::monostate> && requires { a < b; }
		) {
			if constexpr (Op == Less) return static_cast<bool>(a < b);
			else if constexpr (Op == LessEqual) return !static_cast<bool>(b < a);
			else if constexpr (Op == Greater) return static_cast<bool>(b < a);
			else return !static_cast<bool>(a < b);
		}
		else {
			return Unsupported {};
		}
	}
	else if constexpr (Op == Add && std::is_same_v<A, std::string> && std::is_same_v<B, std::string>) {
		return a + b;
	}
	else if constexpr (Op == Add && std::is_same_v<A, VariantArray> && std::is_same_v<B, VariantArray>) {
		VariantArray result = a;
		for (const Variant& value : b) {
			result.push_back(value);
		}
		return result;
	}
	else if constexpr (Op >= Add && Op <= Divide) {
		if constexpr (!arithmetic<A> || !arithmetic<B>) {
			return Unsupported {};
		}
		else if constexpr (Op == Add && requires(PA x, PB y) { x + y; }) {
			return pa() + pb();
		}
		else if constexpr (Op == Subtract && requires(PA x, PB y) { x - y; }) {
			return pa() - pb();
		}
		else if constexpr (Op == Multiply && requires(PA x, PB y) { x * y; }) {
			return pa() * pb();
		}
		else if constexpr (Op == Divide && requires(PA x, PB y) { x / y; }) {
			return pa() / pb();
		}
		else {
			return Unsupported {};
		}
	}
	else if constexpr (Op == Negate) {
		if constexpr (arithmetic<A> && requires { -a; }) return -a;
		else return Unsupported {};
	}
	else if constexpr (Op == Positive) {
		if constexpr (arithmetic<A> && requires { +a; }) return +a;
		else return Unsupported {};
	}
	else if constexpr (Op == Module) {
		if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>) return a % b;
		else if constexpr (number<A> && number<B>) return static_cast<real_t>(std::fmod(pa(), pb()));
		else return Unsupported {};
	}
	else if constexpr (Op == Power) {
		if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>) return int_pow(a, b);
		else if constexpr (number<A> && number<B>) return static_cast<real_t>(std::pow(pa(), pb()));
		else return Unsupported {};
	}
	else if constexpr (Op >= ShiftLeft && Op <= BitXor) {
		if constexpr (!std::is_same_v<A, int> || !std::is_same_v<B, int>) return Unsupported {};
		else if constexpr (Op == ShiftLeft) return static_cast<int>(static_cast<unsigned>(a) << b);
		else if constexpr (Op == ShiftRight) return a >> b;
		else if constexpr (Op == BitAnd) return a & b;
		else if constexpr (Op == BitOr) return a | b;
		else return a ^ b;
	}
	else if constexpr (Op == BitNegate) {
		if constexpr (std::is_same_v<A, int>) return ~a;
		else return Unsupported {};
	}
	else if constexpr (Op == And) {
		return va.booleanize() && vb.booleanize();
	}
	else if constexpr (Op == Or) {
		return va.booleanize() || vb.booleanize();
	}
	else if constexpr (Op == Xor) {
		return va.booleanize() != vb.booleanize();
	}
	else if constexpr (Op == Not) {
		return !va.booleanize();
	}
	else if constexpr (Op == In) {
		if constexpr (std::is_same_v<A, std::string> && std::is_same_v<B, std::string>) {
			return b.find(a) != std::string::npos;
		}
		else if constexpr (std::is_same_v<B, VariantArray>) {
			return std::ranges::any_of(b, [&](const Variant& element) { return element == va; });
		}
		else {
			return Unsupported {};
		}
	}
	else {
		return Unsupported {};
	}
}

template <VariantOperator Op, class A, class B>
using result_t = std::decay_t<decltype(apply<Op>(
		std::declval<const A&>(), std::declval<const B&>(), std::declval<const Variant&>(),
		std::declval<const Variant&>()
))>;

template <VariantOperator Op, class A, class B>
constexpr bool supported = !std::is_same_v<result_t<Op, A, B>, Unsupported> && VariantCompatible<result_t<Op, A, B>>;

// Errors that depend on the operand values rather than their types.
template <VariantOperator Op, class A, class B>
bool check_operands(const A& a, const B& b, std::string& error) {
	using enum VariantOperator;
	if constexpr (std::is_same_v<A, int> && std::is_same_v<B, int>) {
		if constexpr (Op == Divide || Op == Module) {
			if (b == 0) {
				error = Op == Divide ? "Division by zero" : "Modulo by zero";
				return false;
			}
		}
		else if constexpr (Op == ShiftLeft || Op == ShiftRight) {
			if (b < 0 || b >= std::numeric_limits<int>::digits + 1) {
				error = "Invalid shift amount";
				return false;
			}
		}
		else if constexpr (Op == Power) {
			if (b < 0) {
				error = "Negative exponent on an integer power; use a FLOAT operand";
				return false;
			}
		}
	}
	return true;
}

using Evaluator = bool (*)(const Variant& a, const Variant& b, Variant& result, std::string& error);

template <VariantOperator Op, VariantType TA, VariantType TB>
bool evaluate_thunk(const Variant& a, const Variant& b, Variant& result, std::string& error) {
	using A = storage_t<TA>;
	using B = storage_t<TB>;
	const A& x = std::get<A>(a.internal());
	const B& y = std::get<B>(b.internal());
	if (!check_operands<Op>(x, y, error)) {
		return false;
	}
	result = Variant(apply<Op>(x, y, a, b));
	return true;
}

constexpr size_t table_index(VariantOperator op, VariantType a, VariantType b) {
	return (static_cast<size_t>(op) * type_count + static_cast<size_t>(a)) * type_count + static_cast<size_t>(b);
}

template <size_t I>
struct table_entry {
	static constexpr auto op = static_cast<VariantOperator>(I / (type_count * type_count));
	static constexpr auto a = static_cast<VariantType>((I / type_count) % type_count);
	static constexpr auto b = static_cast<VariantType>(I % type_count);
	static constexpr bool valid = supported<op, storage_t<a>, storage_t<b>>;

	static constexpr Evaluator evaluator() {
		if constexpr (valid) {
			return &evaluate_thunk<op, a, b>;
		}
		else {
			return nullptr;
		}
	}

	static constexpr VariantType return_type() {
		if constexpr (valid) {
			return get_variant_type<result_t<op, storage_t<a>, storage_t<b>>>();
		}
		else {
			return VariantType::INVALID;
		}
	}
};

// One table per operator keeps each pack expansion at type_count² entries.
template <VariantOperator Op>
struct operator_table {
	static constexpr size_t base = static_cast<size_t>(Op) * type_count * type_count;

	static constexpr auto evaluators = []<size_t... I>(std::index_sequence<I...>) {
		return std::array<Evaluator, sizeof...(I)> { table_entry<base + I>::evaluator()... };
	}(std::make_index_sequence<type_count * type_count> {});

	static constexpr auto return_types = []<size_t... I>(std::index_sequence<I...>) {
		return std::array<VariantType, sizeof...(I)> { table_entry<base + I>::return_type()... };
	}(std::make_index_sequence<type_count * type_count> {});
};

template <class TElement, template <VariantOperator> class TPick>
constexpr auto build_table() {
	return []<size_t... Op>(std::index_sequence<Op...>) {
		std::array<TElement, operator_count * type_count * type_count> table {};
		size_t offset = 0;
		(
				[&] {
					for (const auto& element : TPick<static_cast<VariantOperator>(Op)>::value) {
						table[offset++] = element;
					}
				}(),
				...
		);
		return table;
	}(std::make_index_sequence<operator_count> {});
}

template <VariantOperator Op>
struct pick_evaluators {
	static constexpr const auto& value = operator_table<Op>::evaluators;
};

template <VariantOperator Op>
struct pick_return_types {
	static constexpr const auto& value = operator_table<Op>::return_types;
};

constexpr auto evaluator_table = build_table<Evaluator, pick_evaluators>();
constexpr auto return_type_table = build_table<VariantType, pick_return_types>();

constexpr bool valid_type(VariantType type) {
	return static_cast<size_t>(type) < type_count;
}

} // namespace

std::string_view Variant::get_operator_name(VariantOperator op) {
	switch (op) {
		case VariantOperator::Equal: return "==";
		case VariantOperator::NotEqual: return "!=";
		case VariantOperator::Less: return "<";
		case VariantOperator::LessEqual: return "<=";
		case VariantOperator::Greater: return ">";
		case VariantOperator::GreaterEqual: return ">=";
		case VariantOperator::Add: return "+";
		case VariantOperator::Subtract: return "-";
		case VariantOperator::Multiply: return "*";
		case VariantOperator::Divide: return "/";
		case VariantOperator::Negate: return "unary-";
		case VariantOperator::Positive: return "unary+";
		case VariantOperator::Module: return "%";
		case VariantOperator::Power: return "**";
		case VariantOperator::ShiftLeft: return "<<";
		case VariantOperator::ShiftRight: return ">>";
		case VariantOperator::BitAnd: return "&";
		case VariantOperator::BitOr: return "|";
		case VariantOperator::BitXor: return "^";
		case VariantOperator::BitNegate: return "~";
		case VariantOperator::And: return "and";
		case VariantOperator::Or: return "or";
		case VariantOperator::Xor: return "xor";
		case VariantOperator::Not: return "not";
		case VariantOperator::In: return "in";
		case VariantOperator::MAX: break;
	}
	return "<invalid operator>";
}

VariantType Variant::get_operator_return_type(VariantOperator op, VariantType a, VariantType b) {
	if (op >= VariantOperator::MAX || !valid_type(a) || !valid_type(b)) {
		return VariantType::INVALID;
	}
	return return_type_table[table_index(op, a, b)];
}

std::expected<Variant, std::string> Variant::evaluate(VariantOperator op, const Variant& a, const Variant& b) {
	if (op >= VariantOperator::MAX || !valid_type(a._type) || !valid_type(b._type)) {
		return std::unexpected("Invalid operator or operand type");
	}
	Evaluator evaluator = evaluator_table[table_index(op, a._type, b._type)];
	if (!evaluator) {
		// Values of unrelated types are simply unequal.
		if (op == VariantOperator::Equal || op == VariantOperator::NotEqual) {
			return Variant(op == VariantOperator::NotEqual);
		}
		return std::unexpected(std::format(
				"Invalid operands '{}' and '{}' for operator '{}'", get_type_name(a._type), get_type_name(b._type),
				get_operator_name(op)
		));
	}
	Variant result;
	std::string error;
	if (!evaluator(a, b, result, error)) {
		return std::unexpected(std::move(error));
	}
	return result;
}

Variant Variant::_evaluate_or_assert(VariantOperator op, const Variant& a, const Variant& b) {
	auto result = evaluate(op, a, b);
	fassert(result.has_value(), result.has_value() ? std::string {} : result.error());
	return result ? std::move(result.value()) : Variant {};
}

} // namespace feather
