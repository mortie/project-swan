#pragma once

#include <string>
#include <typeindex>
#include <functional>
#include <stdint.h>

#include "common.h"
#include "Entity.h"
#include "traits/BodyTrait.h"
#include "swan.capnp.h"

namespace Swan {

class EntityCollection;
class EntitySystemImpl;

class EntityRef {
public:
	EntityRef(): coll_(nullptr), id_(0)
	{}
	EntityRef(const EntityRef &other): coll_(other.coll_), id_(other.id_)
	{}
	EntityRef(EntityCollection *coll, uint64_t id):
		coll_(coll), id_(id)
	{}

	EntityRef &operator=(const EntityRef &other)
	{
		coll_ = other.coll_;
		id_ = other.id_;
		return *this;
	}

	operator bool()
	{
		return exists();
	}

	bool isNil()
	{
		return coll_ == nullptr;
	}

	Entity *operator->()
	{
		return get();
	}

	Entity &operator*()
	{
		return *get();
	}

	bool operator==(const EntityRef &other) const
	{
		return coll_ == other.coll_ && id_ == other.id_;
	}

	template<typename Func>
	EntityRef &then(Func func);

	bool exists();
	Entity *get();
	BodyTrait::Body *getBody();

	template<typename T>
	T *as()
	{
		return dynamic_cast<T *>(get());
	}

	template<typename Trait>
	auto *trait();

	template<typename Trait, typename Func>
	void traitThen(Func func);

	uint64_t id() const
	{
		return id_;
	}

	EntityCollection *collection() const
	{
		return coll_;
	}

	void serialize(proto::EntityRef::Builder w);
	void deserialize(const Context &ctx, proto::EntityRef::Reader r);

private:
	EntityCollection *coll_;
	uint64_t id_;

	friend EntitySystemImpl;
	friend std::hash<EntityRef>;
};

class EntityCollection {
public:
	struct Factory {
		std::string name;
		std::unique_ptr<EntityCollection> (*const create)(std::string name);
	};

	virtual ~EntityCollection() = default;

	template<typename Ent, typename ... Args>
	EntityRef spawn(const Context &ctx, Args && ... args);

	EntityRef currentEntity();

	virtual const std::string &name() = 0;
	virtual std::type_index type() = 0;

	virtual size_t size() = 0;
	virtual Entity *get(uint64_t id) = 0;
	virtual BodyTrait::Body *getBody(uint64_t id) = 0;

	virtual EntityRef spawn(const Context &ctx) = 0;
	virtual EntityRef spawn(const Context &ctx, capnp::Data::Reader data) = 0;
	virtual void update(const Context &ctx, float dt) = 0;
	virtual void tick(const Context &ctx, float dt) = 0;
	virtual void tick2(const Context &ctx, float dt) = 0;
	virtual void draw(const Context &ctx, Cygnet::Renderer &rnd) = 0;
	virtual void erase(const Context &ctx, uint64_t id) = 0;

	virtual void serialize(
		const Context &ctx, proto::EntitySystem::Collection::Builder w) = 0;
	virtual void deserialize(
		const Context &ctx, proto::EntitySystem::Collection::Reader r) = 0;

protected:
	uint64_t currentId_;
};

}

namespace std {

template<>
struct hash<Swan::EntityRef> {
	size_t operator()(const Swan::EntityRef &ref) const
	{
		return
			std::hash<Swan::EntityCollection *>{}(ref.coll_) ^
			std::hash<uint64_t>{}(ref.id_);
	}
};

}
