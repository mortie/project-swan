#pragma once

#include <algorithm>
#include <new>
#include <assert.h>
#include <stddef.h>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Swan {

template<typename T, size_t Capacity>
class ArrayVector {
public:
	using Self = ArrayVector<T, Capacity>;
	using value_type = T;
	using reference = T&;
	using const_reference = const T&;
	using iterator = T*;
	using const_iterator = const T*;
	using difference_type = ptrdiff_t;
	using size_type = size_t;

	constexpr ArrayVector(const Self &other)
	noexcept(std::is_nothrow_constructible_v<T>)
	{
		for (const T &el: other) {
			new (data() + size_) T(el);
			size_ += 1;
		}
	}

	constexpr ArrayVector(Self &&other)
	noexcept(std::is_nothrow_move_constructible_v<T>)
	{
		for (T &el: other) {
			new (data() + size_) T(std::move(el));
			size_ += 1;
		}

		other.clear();
	}

	constexpr ~ArrayVector()
	{
		clear();
	}

	constexpr Self &operator=(const Self &other)
	noexcept(std::is_nothrow_assignable_v<T, T>)
	{
		if (this == &other) {
			return *this;
		}

		clear();
		for (const T &el: other) {
			new (data() + size_) T(el);
			size_ += 1;
		}

		return *this;
	}

	constexpr Self &operator=(Self &&other)
	noexcept(std::is_nothrow_move_assignable_v<T>)
	{
		if (this == &other) {
			return *this;
		}

		clear();
		for (const T &el: other) {
			new (data() + size_) T(std::move(el));
			size_ += 1;
		}

		other.clear();
		return *this;
	}

	constexpr T *data()
	{
		return std::launder(data_);
	}

	constexpr const T *data() const
	{
		return std::launder(data_);
	}

	constexpr size_type size() const
	{
		return size_;
	}

	constexpr size_type capacity() const
	{
		return Capacity;
	}

	constexpr reference operator[](size_t index)
	{
		assert(index < size_);
		return data()[index];
	}

	constexpr const_reference operator[](size_t index) const
	{
		assert(index < size_);
		return data()[index];
	}

	constexpr iterator begin()
	{
		return data();
	}

	constexpr const_iterator begin() const
	{
		return data();
	}

	constexpr const_iterator cbegin() const
	{
		return data();
	}

	constexpr iterator end()
	{
		return data() + size_;
	}

	constexpr const_iterator end() const
	{
		return data() + size_;
	}

	constexpr const_iterator cend() const
	{
		return data() + size_;
	}

	constexpr reference back()
	{
		assert(size_ > 0);
		return data()[size_ - 1];
	}

	constexpr const_reference back() const
	{
		assert(size_ > 0);
		return data()[size_ - 1];
	}

	constexpr reference front()
	{
		assert(size_ > 0);
		return data()[0];
	}

	constexpr const_reference front() const
	{
		assert(size_ > 0);
		return data()[0];
	}

	constexpr reference at(size_type n)
	{
		if (n >= size_) {
			throw std::out_of_range("Out of range");
		}

		return data()[n];
	}

	constexpr const_reference at(size_type n) const
	{
		if (n >= size_) {
			throw std::out_of_range("Out of range");
		}

		return data()[n];
	}

	constexpr void clear()
	{
		while (size_ > 0) {
			data()[size_ - 1].~T();
			size_ -= 1;
		}
	}

	constexpr void push_back(const T &other)
	{
		assert(size_ < Capacity);
		new (data() + size_) T(other);
		size_ += 1;
	}

	constexpr void push_back(T &&other)
	{
		assert(size_ < Capacity);
		new (data() + size_) T(std::move(other));
		size_ += 1;
	}

	constexpr void pop_back()
	{
		assert(size_ > 0);
		data()[size_ - 1].~T();
	}

	template<typename... Args>
	constexpr void emplace_back(Args &&...args)
	{
		assert(size_ < Capacity);
		new (data() + size_) T(std::forward<Args>(args)...);
		size_ += 1;
	}

	constexpr void resize(size_type n)
	{
		while (size_ < n) {
			new (data() + size_) T();
			size_ += 1;
		}

		while (size_ > n) {
			data()[size_ - 1].~T();
			size_ -= 1;
		}
	}

	constexpr void resize(size_type n, const value_type &val)
	{
		while (size_ < n) {
			new (data() + size_) T(val);
			size_ += 1;
		}

		while (size_ > n) {
			data()[size_ - 1].~T();
			size_ -= 1;
		}
	}

private:
	alignas(alignof(T)) unsigned char data_[sizeof(T) * Capacity];
	size_t size_ = 0;
};

}
