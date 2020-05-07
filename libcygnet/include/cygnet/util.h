#pragma once

#include <memory>

/*
 * This file mostly just contains copy-pasted stuff from libswan.
 * I don't love the code duplication, but it feels overkill to have a
 * library which both libcygnet and libswan depends on just for this stuff.
 */

namespace Cygnet {

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

// Take a function, run it when the object goes out of scope
template<void (*Func)()>
class Deferred: NonCopyable {
public:
	Deferred() = default;
	~Deferred() { Func(); }
};


}
