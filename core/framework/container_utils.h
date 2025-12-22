#pragma once

#include <ranges>
#include <utility>
#include <vector>

namespace feather {

template <class T, class U>
void swap_remove(std::vector<T, U>& vec, size_t index) {
	if (index >= vec.size()) {
		return;
	}

	std::swap(vec[index], vec.back());
	vec.pop_back();
}

// Type trait to check if a type is a container
template <typename T>
concept is_container = std::ranges::range<T> && requires { typename T::value_type; };

template <typename T>
concept is_contiguous_container = is_container<T> && std::contiguous_iterator<std::ranges::iterator_t<T>>;

template <typename T>
concept is_associative_container = is_container<T> && requires(T a) {
	typename T::key_type;
	{
		a.find(std::declval<typename T::key_type>())
	} -> std::sentinel_for<std::ranges::iterator_t<T>>;
};

} //namespace feather
