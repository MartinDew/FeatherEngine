module feather.reflection;

import std;

using namespace std::literals;

namespace feather {

Callable::Callable() : _internal_func { nullptr }, _param_amount(0) {
}

Variant Callable::call_on(void* self, std::span<Variant> params) const {
	if (!is_valid()) {
		std::println(std::cout, "Attempting to call an invalid Callable");
		return {};
	}

	if (params.size() != _param_amount) {
		fassert(false,
				std::format(
						"Callable called with incorrect number of parameters. Expected {} and got {}",
						_param_amount,
						params.size()
				));
	}
	fassert(!_is_member || self, "Calling a member function without an instance");
	return _internal_func(self, params);
}

bool Callable::is_valid() const {
	return _internal_func != nullptr;
}

} //namespace feather
