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
	if (!_buffer || _buffer->use_count() > 1) {
		_buffer = std::make_unique<Buffer>(*_buffer);
	}
}

// Constructors
HighLevelArray::HighLevelArray() : _buffer(std::make_unique<Buffer>()) {}

HighLevelArray::HighLevelArray(size_type count) : _buffer(std::make_unique<Buffer>(count)) {}

HighLevelArray::HighLevelArray(size_type count, const Variant& value)
		: _buffer(std::make_unique<Buffer>(count, value)) {}

HighLevelArray::HighLevelArray(std::initializer_list<Variant> ilist) : _buffer(std::make_unique<Buffer>(ilist)) {}

// Explicit instantiations for common iterator types
template HighLevelArray::HighLevelArray(Variant*, Variant*);
template HighLevelArray::HighLevelArray(const Variant*, const Variant*);

// Copy and move
HighLevelArray::HighLevelArray(const HighLevelArray& other) : _buffer(std::make_unique<Buffer>(*other._buffer)) {}

HighLevelArray::HighLevelArray(HighLevelArray&& other) noexcept : _buffer(std::move(other._buffer)) {}

HighLevelArray& HighLevelArray::operator=(const HighLevelArray& other) {
	if (this != &other) {
		_buffer = std::make_unique<Buffer>(*other._buffer);
	}
	return *this;
}

HighLevelArray& HighLevelArray::operator=(HighLevelArray&& other) noexcept {
	if (this != &other) {
		_buffer = std::move(other._buffer);
	}
	return *this;
}

HighLevelArray& HighLevelArray::operator=(std::initializer_list<Variant> ilist) {
	_buffer = std::make_unique<Buffer>(ilist);
	return *this;
}

HighLevelArray::~HighLevelArray() = default;

// Element access
Variant& HighLevelArray::at(size_type pos) {
	ensure_unique();
	return _buffer->data.at(pos);
}

const Variant& HighLevelArray::at(size_type pos) const { return _buffer->data.at(pos); }

Variant& HighLevelArray::operator[](size_type pos) {
	ensure_unique();
	return _buffer->data[pos];
}

const Variant& HighLevelArray::operator[](size_type pos) const { return _buffer->data[pos]; }

Variant& HighLevelArray::front() {
	ensure_unique();
	return _buffer->data.front();
}

const Variant& HighLevelArray::front() const { return _buffer->data.front(); }

Variant& HighLevelArray::back() {
	ensure_unique();
	return _buffer->data.back();
}

const Variant& HighLevelArray::back() const { return _buffer->data.back(); }

// Iterators
HighLevelArray::iterator HighLevelArray::begin() {
	ensure_unique();
	return iterator(&(*_buffer->data.begin()));
}

HighLevelArray::const_iterator HighLevelArray::begin() const noexcept {
	return const_iterator(&(*_buffer->data.begin()));
}

HighLevelArray::const_iterator HighLevelArray::cbegin() const noexcept {
	return const_iterator(&(*_buffer->data.cbegin()));
}

HighLevelArray::iterator HighLevelArray::end() {
	ensure_unique();
	return iterator(&(*_buffer->data.begin()) + _buffer->data.size());
}

HighLevelArray::const_iterator HighLevelArray::end() const noexcept {
	return const_iterator(&(*_buffer->data.begin()) + _buffer->data.size());
}

HighLevelArray::const_iterator HighLevelArray::cend() const noexcept {
	return const_iterator(&(*_buffer->data.cbegin()) + _buffer->data.size());
}

// Capacity
bool HighLevelArray::empty() const noexcept { return _buffer->data.empty(); }

HighLevelArray::size_type HighLevelArray::size() const noexcept { return _buffer->data.size(); }

HighLevelArray::size_type HighLevelArray::capacity() const noexcept { return _buffer->data.capacity(); }

void HighLevelArray::reserve(size_type new_cap) {
	ensure_unique();
	_buffer->data.reserve(new_cap);
}

void HighLevelArray::shrink_to_fit() {
	ensure_unique();
	_buffer->data.shrink_to_fit();
}

// Modifiers
void HighLevelArray::clear() noexcept {
	ensure_unique();
	_buffer->data.clear();
}

void HighLevelArray::push_back(const Variant& value) {
	ensure_unique();
	_buffer->data.push_back(value);
}

void HighLevelArray::push_back(Variant&& value) {
	ensure_unique();
	_buffer->data.push_back(std::move(value));
}

template <typename... Args>
Variant& HighLevelArray::emplace_back(Args&&... args) {
	ensure_unique();
	return _buffer->data.emplace_back(std::forward<Args>(args)...);
}

// Explicit instantiation for common cases
template Variant& HighLevelArray::emplace_back();
template Variant& HighLevelArray::emplace_back(Variant&&);
template Variant& HighLevelArray::emplace_back(const Variant&);

void HighLevelArray::pop_back() {
	ensure_unique();
	_buffer->data.pop_back();
}

void HighLevelArray::resize(size_type count) {
	ensure_unique();
	_buffer->data.resize(count);
}

void HighLevelArray::resize(size_type count, const Variant& value) {
	ensure_unique();
	_buffer->data.resize(count, value);
}

// COW-specific
bool HighLevelArray::is_shared() const noexcept { return _buffer->data.use_count() > 1; }

size_t HighLevelArray::use_count() const noexcept { return _buffer->use_count(); }

// Comparison
bool HighLevelArray::operator==(const HighLevelArray& other) const {
	if (_buffer == other._buffer)
		return true;
	return _buffer->data == other._buffer->data;
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