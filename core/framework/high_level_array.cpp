#include "high_level_array.h"
#include "cow_vector.h"
#include "variant.h"
#include <stdexcept>

namespace feather {

// Implementation class wraps CowVector<Variant>
class HighLevelArray::Buffer {
public:
	CowVector<Variant> data;

	Buffer() = default;
	explicit Buffer(size_t count) : data(count) {}
	Buffer(size_t count, const Variant& value) : data(count, value) {}
	Buffer(std::initializer_list<Variant> ilist) : data(ilist) {}

	template <typename InputIt>
	Buffer(InputIt first, InputIt last) : data(first, last) {}

	size_t use_count() const { return data.use_count(); }
};

void HighLevelArray::ensure_unique() {
	if (impl_ && impl_->use_count() > 1) {
		impl_ = std::make_unique<Buffer>(*impl_);
	}
}

// Constructors
HighLevelArray::HighLevelArray() : impl_(std::make_unique<Buffer>()) {}

HighLevelArray::HighLevelArray(size_type count) : impl_(std::make_unique<Buffer>(count)) {}

HighLevelArray::HighLevelArray(size_type count, const Variant& value) : impl_(std::make_unique<Buffer>(count, value)) {}

HighLevelArray::HighLevelArray(std::initializer_list<Variant> ilist) : impl_(std::make_unique<Buffer>(ilist)) {}

template <typename InputIt>
HighLevelArray::HighLevelArray(InputIt first, InputIt last) : impl_(std::make_unique<Buffer>(first, last)) {}

// Explicit instantiations for common iterator types
template HighLevelArray::HighLevelArray(Variant*, Variant*);
template HighLevelArray::HighLevelArray(const Variant*, const Variant*);

// Copy and move
HighLevelArray::HighLevelArray(const HighLevelArray& other) : impl_(std::make_unique<Buffer>(*other.impl_)) {}

HighLevelArray::HighLevelArray(HighLevelArray&& other) noexcept : impl_(std::move(other.impl_)) {}

HighLevelArray& HighLevelArray::operator=(const HighLevelArray& other) {
	if (this != &other) {
		impl_ = std::make_unique<Buffer>(*other.impl_);
	}
	return *this;
}

HighLevelArray& HighLevelArray::operator=(HighLevelArray&& other) noexcept {
	if (this != &other) {
		impl_ = std::move(other.impl_);
	}
	return *this;
}

HighLevelArray& HighLevelArray::operator=(std::initializer_list<Variant> ilist) {
	impl_ = std::make_unique<Buffer>(ilist);
	return *this;
}

HighLevelArray::~HighLevelArray() = default;

// Element access
Variant& HighLevelArray::at(size_type pos) {
	ensure_unique();
	return impl_->data.at(pos);
}

const Variant& HighLevelArray::at(size_type pos) const { return impl_->data.at(pos); }

Variant& HighLevelArray::operator[](size_type pos) {
	ensure_unique();
	return impl_->data[pos];
}

const Variant& HighLevelArray::operator[](size_type pos) const { return impl_->data[pos]; }

Variant& HighLevelArray::front() {
	ensure_unique();
	return impl_->data.front();
}

const Variant& HighLevelArray::front() const { return impl_->data.front(); }

Variant& HighLevelArray::back() {
	ensure_unique();
	return impl_->data.back();
}

const Variant& HighLevelArray::back() const { return impl_->data.back(); }

// Iterators
HighLevelArray::iterator HighLevelArray::begin() {
	ensure_unique();
	return iterator(&(*impl_->data.begin()));
}

HighLevelArray::const_iterator HighLevelArray::begin() const noexcept {
	return const_iterator(&(*impl_->data.begin()));
}

HighLevelArray::const_iterator HighLevelArray::cbegin() const noexcept {
	return const_iterator(&(*impl_->data.cbegin()));
}

HighLevelArray::iterator HighLevelArray::end() {
	ensure_unique();
	return iterator(&(*impl_->data.begin()) + impl_->data.size());
}

HighLevelArray::const_iterator HighLevelArray::end() const noexcept {
	return const_iterator(&(*impl_->data.begin()) + impl_->data.size());
}

HighLevelArray::const_iterator HighLevelArray::cend() const noexcept {
	return const_iterator(&(*impl_->data.cbegin()) + impl_->data.size());
}

// Capacity
bool HighLevelArray::empty() const noexcept { return impl_->data.empty(); }

HighLevelArray::size_type HighLevelArray::size() const noexcept { return impl_->data.size(); }

HighLevelArray::size_type HighLevelArray::capacity() const noexcept { return impl_->data.capacity(); }

void HighLevelArray::reserve(size_type new_cap) {
	ensure_unique();
	impl_->data.reserve(new_cap);
}

void HighLevelArray::shrink_to_fit() {
	ensure_unique();
	impl_->data.shrink_to_fit();
}

// Modifiers
void HighLevelArray::clear() noexcept {
	ensure_unique();
	impl_->data.clear();
}

void HighLevelArray::push_back(const Variant& value) {
	ensure_unique();
	impl_->data.push_back(value);
}

void HighLevelArray::push_back(Variant&& value) {
	ensure_unique();
	impl_->data.push_back(std::move(value));
}

template <typename... Args>
Variant& HighLevelArray::emplace_back(Args&&... args) {
	ensure_unique();
	return impl_->data.emplace_back(std::forward<Args>(args)...);
}

// Explicit instantiation for common cases
template Variant& HighLevelArray::emplace_back();
template Variant& HighLevelArray::emplace_back(Variant&&);
template Variant& HighLevelArray::emplace_back(const Variant&);

void HighLevelArray::pop_back() {
	ensure_unique();
	impl_->data.pop_back();
}

void HighLevelArray::resize(size_type count) {
	ensure_unique();
	impl_->data.resize(count);
}

void HighLevelArray::resize(size_type count, const Variant& value) {
	ensure_unique();
	impl_->data.resize(count, value);
}

// COW-specific
bool HighLevelArray::is_shared() const noexcept { return impl_->data.use_count() > 1; }

size_t HighLevelArray::use_count() const noexcept { return impl_->use_count(); }

// Comparison
bool HighLevelArray::operator==(const HighLevelArray& other) const {
	if (impl_ == other.impl_)
		return true;
	return impl_->data == other.impl_->data;
}

bool HighLevelArray::operator!=(const HighLevelArray& other) const { return !(*this == other); }

// Iterator implementations
HighLevelArray::iterator::iterator(Variant* ptr) : ptr_(ptr) {}

Variant& HighLevelArray::iterator::operator*() const { return *ptr_; }
Variant* HighLevelArray::iterator::operator->() const { return ptr_; }

HighLevelArray::iterator& HighLevelArray::iterator::operator++() {
	++ptr_;
	return *this;
}

HighLevelArray::iterator HighLevelArray::iterator::operator++(int) {
	iterator tmp = *this;
	++ptr_;
	return tmp;
}

HighLevelArray::iterator& HighLevelArray::iterator::operator--() {
	--ptr_;
	return *this;
}

HighLevelArray::iterator HighLevelArray::iterator::operator--(int) {
	iterator tmp = *this;
	--ptr_;
	return tmp;
}

HighLevelArray::iterator& HighLevelArray::iterator::operator+=(difference_type n) {
	ptr_ += n;
	return *this;
}

HighLevelArray::iterator& HighLevelArray::iterator::operator-=(difference_type n) {
	ptr_ -= n;
	return *this;
}

HighLevelArray::iterator HighLevelArray::iterator::operator+(difference_type n) const { return iterator(ptr_ + n); }

HighLevelArray::iterator HighLevelArray::iterator::operator-(difference_type n) const { return iterator(ptr_ - n); }

ptrdiff_t HighLevelArray::iterator::operator-(const iterator& other) const { return ptr_ - other.ptr_; }

Variant& HighLevelArray::iterator::operator[](difference_type n) const { return ptr_[n]; }

bool HighLevelArray::iterator::operator==(const iterator& other) const { return ptr_ == other.ptr_; }

bool HighLevelArray::iterator::operator!=(const iterator& other) const { return ptr_ != other.ptr_; }

bool HighLevelArray::iterator::operator<(const iterator& other) const { return ptr_ < other.ptr_; }

bool HighLevelArray::iterator::operator<=(const iterator& other) const { return ptr_ <= other.ptr_; }

bool HighLevelArray::iterator::operator>(const iterator& other) const { return ptr_ > other.ptr_; }

bool HighLevelArray::iterator::operator>=(const iterator& other) const { return ptr_ >= other.ptr_; }

// const_iterator implementations
HighLevelArray::const_iterator::const_iterator(const Variant* ptr) : ptr_(ptr) {}

HighLevelArray::const_iterator::const_iterator(const iterator& it) : ptr_(&(*it)) {}

const Variant& HighLevelArray::const_iterator::operator*() const { return *ptr_; }
const Variant* HighLevelArray::const_iterator::operator->() const { return ptr_; }

HighLevelArray::const_iterator& HighLevelArray::const_iterator::operator++() {
	++ptr_;
	return *this;
}

HighLevelArray::const_iterator HighLevelArray::const_iterator::operator++(int) {
	const_iterator tmp = *this;
	++ptr_;
	return tmp;
}

HighLevelArray::const_iterator& HighLevelArray::const_iterator::operator--() {
	--ptr_;
	return *this;
}

HighLevelArray::const_iterator HighLevelArray::const_iterator::operator--(int) {
	const_iterator tmp = *this;
	--ptr_;
	return tmp;
}

HighLevelArray::const_iterator& HighLevelArray::const_iterator::operator+=(difference_type n) {
	ptr_ += n;
	return *this;
}

HighLevelArray::const_iterator& HighLevelArray::const_iterator::operator-=(difference_type n) {
	ptr_ -= n;
	return *this;
}

HighLevelArray::const_iterator HighLevelArray::const_iterator::operator+(difference_type n) const {
	return const_iterator(ptr_ + n);
}

HighLevelArray::const_iterator HighLevelArray::const_iterator::operator-(difference_type n) const {
	return const_iterator(ptr_ - n);
}

ptrdiff_t HighLevelArray::const_iterator::operator-(const const_iterator& other) const { return ptr_ - other.ptr_; }

const Variant& HighLevelArray::const_iterator::operator[](difference_type n) const { return ptr_[n]; }

bool HighLevelArray::const_iterator::operator==(const const_iterator& other) const { return ptr_ == other.ptr_; }

bool HighLevelArray::const_iterator::operator!=(const const_iterator& other) const { return ptr_ != other.ptr_; }

bool HighLevelArray::const_iterator::operator<(const const_iterator& other) const { return ptr_ < other.ptr_; }

bool HighLevelArray::const_iterator::operator<=(const const_iterator& other) const { return ptr_ <= other.ptr_; }

bool HighLevelArray::const_iterator::operator>(const const_iterator& other) const { return ptr_ > other.ptr_; }

bool HighLevelArray::const_iterator::operator>=(const const_iterator& other) const { return ptr_ >= other.ptr_; }

} // namespace feather