#pragma once

#include <optional>
#include <functional>
#include <memory>
#include <chrono>
#include <type_traits>
#include <string>
#include <cstddef>
#include <cstdint>

namespace Swan {

inline uint32_t random(uint32_t x) {
	x ^= 0x55555555u;
	x = ((x >> 16u) ^ x) * 0x45d9f3bu;
	x = ((x >> 16u) ^ x) * 0x45d9f3bu;
	x = (x >> 16u) ^ x;
	return x;
}

// Inherit from this class to make a class non-copyable
class NonCopyable {
public:
	NonCopyable(const NonCopyable &) = delete;
	NonCopyable(NonCopyable &&) noexcept = default;
	NonCopyable &operator=(const NonCopyable &) = delete;
	NonCopyable &operator=(NonCopyable &&) = default;

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

template <typename F>
class Defer {
public:
	Defer(F f)
		: f_(f)
	{
	}
	~Defer() { f_(); }

private:
	F f_;
};

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x) DEFER_2(x, __COUNTER__)

/// Run some code after the end of the block.
#define defer(code) auto DEFER_3(_defer_) = ::Swan::Defer([&]() { code; })

inline struct ResultOk {} Ok;
inline struct ResultErr {} Err;

// Result type for potential errors
template<typename T, typename Err = std::string>
class Result {
public:
	Result(ResultOk, T &&val): isOk_(true), v_(ResultOk{}, std::move(val)) {}
	Result(ResultErr, Err &&err): isOk_(false), v_(ResultErr{}, std::move(err)) {}

	Result(const Result &other): isOk_(other.isOk_) {
		if (isOk_) {
			new (&v_.val) T(other.v_.val);
		} else {
			new (&v_.err) T(other.v_.err);
		}
	}

	Result(Result &&other): isOk_(other.isOk_) {
		if (other.isOk_) {
			new (&v_.val) T(std::move(other.v_.val));
		} else {
			new (&v_.err) Err(std::move(other.v_.err));
		}
	}

	~Result() {
		destroy();
	}

	Result<T, Err> &operator=(const Result<T, Err> &other) {
		destroy();
		isOk_ = other.isOk_;
		if (isOk_) {
			new (&v_.val) T(other.v_.val);
		} else {
			new (&v_.err) Err(other.v_.err);
		}
		return *this;
	}

	Result<T, Err> &operator=(Result<T, Err> &&other) {
		destroy();
		isOk_ = other.isOk_;
		if (other.isOk_) {
			new (&v_.val) T(std::move(other.v_.val));
		} else {
			new (&v_.err) Err(std::move(other.v_.err));
		}
		return *this;
	}

	explicit operator bool() { return isOk_; }
	bool isOk() { return isOk_; }

	Err &err() { return v_.err; }
	T &value() { return v_.val; }

	T *operator->() {
		return &v_.val;
	}

	T &operator*() {
		return v_.val;
	}

private:
	void destroy() {
		if (isOk_) {
			v_.val.~T();
		} else {
			v_.err.~Err();
		}
	}

	bool isOk_;
	union U {
		U() {}
		U(ResultOk, T &&val): val(std::move(val)) {}
		U(ResultOk, const T &val): val(val) {}
		U(ResultErr, Err &&err): err(std::move(err)) {}
		U(ResultErr, const Err &err): err(err) {}
		~U() {}
		T val;
		Err err;
	} v_;
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

}
