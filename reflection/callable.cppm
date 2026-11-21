export module feather.reflection:callable;

import std.compat;
import feather.framework;
import :variant;

export namespace feather {

class FEATHER_REFLECTION_API Callable {
	std::function<Variant(std::span<Variant>)> _internal_func;
	uint8_t _param_amount;

	// A non-const T& parameter binds to the instance inside the Variant; any other parameter gets a converted copy.
	template <class TArg>
	using _arg_storage_t = std::conditional_t<
			std::is_lvalue_reference_v<TArg> && !std::is_const_v<std::remove_reference_t<TArg>>,
			TArg,
			std::decay_t<TArg>>;

	template <class TArg>
	static _arg_storage_t<TArg> _convert(Variant& param) {
		if constexpr (std::is_reference_v<_arg_storage_t<TArg>>) {
			return static_cast<TArg>(param);
		}
		else {
			return param.as<std::decay_t<TArg>>().value();
		}
	}

public:
	Callable();
	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(std::function<TRet(TArgs...)> func)
			: _internal_func { [func](std::span<Variant> params) {
				size_t i = 0;
				std::tuple<_arg_storage_t<TArgs>...> converted_args = { _convert<TArgs>(params[i++])... };
				if constexpr (std::is_void_v<TRet>) {
					std::apply(func, converted_args);
					return Variant();
				}
				else {
					TRet result = std::apply(func, converted_args);
					return Variant(std::move(result));
				}
			} }
			, _param_amount { sizeof...(TArgs) } {}

	template <class TRet, class... TArgs>
		requires(VariantCompatible<std::decay_t<TRet>>) && (VariantCompatible<std::decay_t<TArgs>> && ...)
	Callable(TRet (*func_ptr)(TArgs...)) : Callable(std::function<TRet(TArgs...)>(func_ptr)) {}

	Callable(Callable&&) = default;
	Callable(const Callable&) = default;
	Callable& operator=(const Callable&) = default;
	Callable& operator=(Callable&&) = default;

	Variant call(std::span<Variant> params) const;

	Variant call(auto&&... args) const {
		if constexpr (sizeof...(args) == 0) {
			return call(std::span<Variant>());
		}
		else {
			Variant params[] = { args... };
			return call(std::span<Variant>(params, sizeof...(args)));
		}
	}

	bool is_valid() const;
};

} // namespace feather
