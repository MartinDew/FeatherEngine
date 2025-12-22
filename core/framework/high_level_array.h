#pragma once
#include "container_utils.h"
#include "cow_vector.h"

namespace feather {

class HighLevelArray : public CowVector<class Variant> {
public:
	HighLevelArray() = default;
	HighLevelArray(const HighLevelArray& other) = default;
	HighLevelArray(HighLevelArray&& other) noexcept = default;
	HighLevelArray& operator=(const HighLevelArray& other) = default;
	HighLevelArray& operator=(HighLevelArray&& other) noexcept = default;
	HighLevelArray(std::initializer_list<Variant> ilist) : CowVector<Variant>(ilist) {}
	// template <class T>
	// HighLevelArray(const T& container) {
	// 	static_assert(is_contiguous_container<T>, "HighLevelArray can only be constructed from contiguous containers");
	// 	this->reserve(std::size(container));
	// 	for (const auto& item : container) {
	// 		this->push_back(Variant(item));
	// 	}
	// }

	template <class It>
	HighLevelArray(It begin, It end) : CowVector(begin, end) {}
};

} //namespace feather