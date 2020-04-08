#pragma once

#include <functional>

#include "util.h"
#include "SlotVector.h"

namespace Swan {

// So that EventListener doesn't have to be a template
class EventEmitterInterface {
public:
	virtual void unsubscribe(size_t id) = 0;
};

class EventListener: NonCopyable {
public:
	EventListener() {}

	EventListener(EventEmitterInterface *emitter, size_t id):
		emitter_(emitter), id_(id) {}

	EventListener(EventListener &&other):
			emitter_(other.emitter_), id_(other.id_) {
		other.emitter_ = nullptr;
	}

	EventListener &operator=(EventListener &&other) {
		emitter_ = other.emitter_;
		id_ = other.id_;
		other.emitter_ = nullptr;
		return *this;
	}

	~EventListener() {
		if (emitter_)
			emitter_->unsubscribe(id_);
	}

	void unsubscribe() {
		if (emitter_) {
			emitter_->unsubscribe(id_);
			emitter_ = nullptr;
		}
	}

private:
	EventEmitterInterface *emitter_ = nullptr;
	size_t id_;
};

template<typename... Evt>
class EventEmitter: public EventEmitterInterface {
public:
	using Callback = std::function<void(Evt...)>;

	void emit(Evt... evt) {
		for (auto &cb: callbacks_)
			cb(evt...);
	}

	EventListener subscribe(Callback cb) {
		size_t id = callbacks_.insert(std::move(cb));
		return EventListener(this, id);
	}

	void unsubscribe(size_t id) {
		callbacks_.erase(id);
	}

private:
	SlotVector<Callback, SlotVectorDefaultSentinel<nullptr_t>> callbacks_;
};

}
