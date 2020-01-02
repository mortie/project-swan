 #pragma once

#include <optional>
#include <functional>
#include <memory>
#include <chrono>

namespace Swan {

// Inherit from this class to make a class non-copyable
class NonCopyable {
protected:
	NonCopyable() = default;
	NonCopyable(NonCopyable &&) = default;

	NonCopyable(const NonCopyable &) = delete;
	NonCopyable &operator=(const NonCopyable &) = delete;
};

template<typename T, typename Del = void (*)(T *)>
using RaiiPtr = std::unique_ptr<T, Del>;

template<typename T, typename Del>
RaiiPtr<T, Del> makeRaiiPtr(T *val, Del d) {
	return std::unique_ptr<T, Del>(val, d);
}

template<typename Func>
class Deferred: NonCopyable {
public:
	Deferred(Func func): func_(func) {}
	Deferred(Deferred &&def) noexcept: func_(def.func_) { def.active_ = false; }
	~Deferred() { if (active_) func_(); }

	Deferred &operator=(Deferred &&def) noexcept {
		func_ = def.func_;
		def.active_ = false;
		return *this;
	}

private:
	Func func_;
	bool active_ = true;
};

template<typename Func>
Deferred<Func> makeDeferred(Func func) {
	return Deferred(func);
}

// Ret can't be a reference, because C++ doesn't support optional<T&>.
template<typename Ret, typename Func = std::function<std::optional<Ret>()>>
class Iter {
public:
	class It {
	public:
		It(std::optional<Ret> next, Func &func): next_(std::move(next)), func_(func) {}

		bool operator==(It &other) {
			return next_ == std::nullopt && other.next_ == std::nullopt;
		}

		bool operator!=(It &other) {
			return !(*this == other);
		}

		void operator++() {
			next_ = std::move(func_());
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
		first++;
		return r;
	};

	return Iter<RetT, decltype(l)>(l);
}

}
