
 #pragma once

#include <optional>
#include <functional>
#include <memory>
#include <chrono>
#include <type_traits>
#include <stddef.h>

namespace Swan {

// Inherit from this class to make a class non-copyable
class NonCopyable {
public:
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
	NonCopyable(NonCopyable &&) noexcept = default;

protected:
	NonCopyable() = default;
	~NonCopyable() = default;
};

// Take a deleter function, turn it into a class with an operator() for unique_ptr
template<typename T, void (*Func)(T *)>
class CPtrDeleter {
public:
	void operator()(T *ptr) { Func(ptr); }
};

// This is just a bit nicer to use than using unique_ptr directly
template<typename T, void (*Func)(T *)>
using CPtr = std::unique_ptr<T, CPtrDeleter<T, Func>>;

// Take a function, run it when the object goes out of scope
template<void (*Func)()>
class Deferred: NonCopyable {
public:
	Deferred() = default;
	~Deferred() { Func(); }
};

// Calling begin/end is stupid...
template<typename T>
auto callBegin(T &v) {
	using namespace std;
	return begin(v);
}
template<typename T>
auto callEnd(T &v) {
	using namespace std;
	return end(v);
}

// Ret can't be a reference, because C++ doesn't support optional<T&>.
template<typename Ret, typename Func = std::function<std::optional<Ret>()>>
class Iter {
public:
	class It {
	public:
		It(std::optional<Ret> next, Func &func): next_(std::move(next)), func_(func) {}

		bool operator==(const It &other) const {
			return next_ == std::nullopt && other.next_ == std::nullopt;
		}

		bool operator!=(const It &other) const {
			return !(*this == other);
		}

		void operator++() {
			auto val(func_());
			if (val)
				next_.emplace(std::move(*val));
			else
				next_.reset();
		}

		Ret operator*() {
			return std::move(*next_);
		}

	private:
		std::optional<Ret> next_;
		Func &func_;
	};

	Iter(Func func): func_(func) {}

	operator Iter<Ret, std::function<std::optional<Ret>()>>() {
		return Iter<Ret, std::function<std::optional<Ret>()>>(func_);
	}

	It begin() {
		return It(func_(), func_);
	}

	It end() {
		return It(std::nullopt, func_);
	}

private:
	Func func_;
};

template<typename InputIterator, typename Func>
auto map(InputIterator first, InputIterator last, Func func) {
	using RetT = decltype(func(*first));

	auto l = [=]() mutable -> std::optional<RetT> {
		if (first == last)
			return std::nullopt;

		RetT r = func(*first);
		++first;
		return r;
	};

	return Iter<RetT, decltype(l)>(l);
}

template<typename Range, typename Func>
auto map(Range &rng, Func func) {
	return map(callBegin(rng), callEnd(rng), func);
}

template<typename InputIterator, typename Func>
auto filter(InputIterator first, InputIterator last, Func pred) {
	using RetT = std::remove_reference_t<decltype(*first)>;

	auto l = [=]() mutable -> std::optional<RetT> {
		if (first == last)
			return std::nullopt;

		while (!pred(*first)) {
			++first;
			if (first == last)
				return std::nullopt;
		}

		RetT r = *first;
		++first;
		return r;
	};

	return Iter<RetT, decltype(l)>(l);
}

template<typename Range, typename Func>
auto filter(Range &rng, Func func) {
	return filter(callBegin(rng), callEnd(rng), func);
}

template<typename InputIterator, typename Func>
auto mapFilter(InputIterator first, InputIterator last, Func func) {
	using RetT = std::remove_reference_t<decltype(*func(*first))>;

	auto l = [=]() mutable -> std::optional<RetT> {
		if (first == last)
			return std::nullopt;

		std::optional<RetT> r;
		while ((r = func(*first)) == std::nullopt) {
			++first;
			if (first == last)
				return std::nullopt;
		}

		++first;
		return r;
	};

	return Iter<RetT, decltype(l)>(l);
}

template<typename Range, typename Func>
auto mapFilter(Range &rng, Func func) {
	return mapFilter(callBegin(rng), callEnd(rng), func);
}

}
