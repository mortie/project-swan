#pragma once

#include <functional>
#include <vector>
#include <unordered_map>

#include <swan/util.h>
#include "log.h"

namespace Swan {

// So that EventListener doesn't have to be a template
class EventEmitterInterface {
public:
	virtual void unsubscribe(size_t id) = 0;

protected:
	~EventEmitterInterface() = default;
};

class EventListener: NonCopyable {
public:
	EventListener()
	{}

	EventListener(EventEmitterInterface *emitter, uint64_t id):
		emitter_(emitter), id_(id)
	{}

	EventListener(EventListener &&other) noexcept:
		emitter_(other.emitter_), id_(other.id_)
	{
		other.emitter_ = nullptr;
	}

	EventListener &operator=(EventListener &&other) noexcept
	{
		emitter_ = other.emitter_;
		id_ = other.id_;
		other.emitter_ = nullptr;
		return *this;
	}

	~EventListener()
	{
		unsubscribe();
	}

	void unsubscribe()
	{
		if (emitter_) {
			emitter_->unsubscribe(id_);
			emitter_ = nullptr;
		}
	}

private:
	EventEmitterInterface *emitter_ = nullptr;
	uint64_t id_;
};

template<typename ... Evt>
class EventEmitter final: public EventEmitterInterface {
public:
	using Callback = std::function<void (Evt...)>;

	void emit(Evt... evt)
	{
		for (auto &cb: callbacks_) {
			cb(evt ...);
		}
	}

	EventListener subscribe(Callback &&cb)
	{
		uint64_t id = nextId_++;
		size_t index = callbacks_.size();

		ids_.push_back(id);
		callbacks_.push_back(std::move(cb));
		idToIndex_[id] = index;
		return EventListener(this, id);
	}

	void unsubscribe(size_t id)
	{
		auto indexIt = idToIndex_.find(id);

		if (indexIt == idToIndex_.end()) {
			Swan::warn << "Attempt to unsubscribe non-existent event " << id;
			return;
		}

		size_t index = indexIt->second;
		if (index == callbacks_.size() - 1) {
			callbacks_.pop_back();
			ids_.pop_back();
			return;
		}

		callbacks_[index] = std::move(callbacks_.back());
		ids_[index] = ids_.back();
		callbacks_.pop_back();
		ids_.pop_back();
		idToIndex_.erase(id);
		idToIndex_[ids_[index]] = index;
	}

private:
	uint64_t nextId_ = 0;
	std::vector<Callback> callbacks_;
	std::vector<int64_t> ids_;
	std::unordered_map<uint64_t, size_t> idToIndex_;
};

}
