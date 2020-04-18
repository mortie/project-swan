#pragma once

#include <vector>
#include <string>
#include <typeindex>

#include "common.h"
#include "Entity.h"
#include "SlotVector.h"
#include "SmallOptional.h"

namespace Swan {

class EntityCollection;

class EntityRef {
public:
	EntityRef() = default;
	EntityRef(EntityCollection *coll, size_t index, size_t generation):
		coll_(coll), index_(index), generation_(generation) {}

	template<typename Func>
	EntityRef &then(Func func);

	bool hasValue();
	Entity *get();

private:
	EntityCollection *coll_;
	size_t index_;
	size_t generation_;
};

class EntityCollection {
public:
	struct Factory {
		const std::string name;
		std::unique_ptr<EntityCollection> (*const create)(std::string name);
	};

	template<typename Ent>
	struct EntWrapper {
		size_t generation;
		Ent ent;
		bool operator==(const EntWrapper &other) const {
			return generation == other.generation;
		}
	};

	template<typename Ent>
	struct OptionalPolicy {
		static void setEmpty(unsigned char *ptr) {
			((EntWrapper<Ent> *)ptr)->generation = ~0ull;
		}
		static bool isEmpty(const unsigned char *ptr) {
			return ((EntWrapper<Ent> *)ptr)->generation == ~0ull;
		}
	};

	template<typename Ent>
	using OptEnt = SmallOptional<EntWrapper<Ent>, OptionalPolicy<Ent>>;

	virtual ~EntityCollection() = default;

	template<typename Ent, typename... Args>
	EntityRef spawn(Args&&... args);

	virtual const std::string &name() = 0;
	virtual std::type_index type() = 0;

	virtual size_t size() = 0;
	virtual Entity *get(size_t idx) = 0;
	virtual Entity *get(size_t idx, size_t version) = 0;

	virtual EntityRef spawn(const Context &ctx, const Entity::PackObject &obj) = 0;
	virtual void update(const Context &ctx, float dt) = 0;
	virtual void tick(const Context &ctx, float dt) = 0;
	virtual void draw(const Context &ctx, Win &win) = 0;

private:
	virtual void *getEntityVector() = 0;
	virtual size_t nextGeneration() = 0;
};

template<typename Ent>
class EntityCollectionImpl: public EntityCollection {
public:
	EntityCollectionImpl(std::string name): name_(std::move(name)) {}

	size_t size() override { return entities_.size(); }
	Entity *get(size_t idx) override { return &entities_[idx]->ent; }
	Entity *get(size_t idx, size_t generation) override;

	const std::string &name() override { return name_; }
	std::type_index type() override { return typeid(Ent); }

	EntityRef spawn(const Context &ctx, const Entity::PackObject &obj) override;
	void update(const Context &ctx, float dt) override;
	void tick(const Context &ctx, float dt) override;
	void draw(const Context &ctx, Win &win) override;

private:
	void *getEntityVector() override { return (void *)&entities_; }
	size_t nextGeneration() override { return generation_++; }

	const std::string name_;
	SlotVector<EntityCollection::OptEnt<Ent>> entities_;
	size_t generation_ = 0;
};

/*
 * EntityRef
 */

template<typename Func>
inline EntityRef &EntityRef::then(Func func) {
	Entity *ent = coll_->get(index_, generation_);
	if (ent != nullptr)
		func(ent);

	return *this;
}

inline Entity *EntityRef::get() {
	return coll_->get(index_, generation_);
}

inline bool EntityRef::hasValue() {
	return coll_->get(index_, generation_) != nullptr;
}

/*
 * EntityCollection
 */

template<typename Ent, typename... Args>
inline EntityRef EntityCollection::spawn(Args&&... args) {
	auto entities = (SlotVector<OptEnt<Ent>> *)getEntityVector();

	size_t generation = nextGeneration();
	size_t idx = entities->emplace(EntWrapper<Ent>{
		generation, std::move(Ent(std::forward<Args>(args)...)) });

	return { this, idx, generation };
}

/*
 * EntityCollectionImpl
 */

template<typename Ent>
inline Entity *EntityCollectionImpl<Ent>::get(size_t idx, size_t generation) {
	if (idx >=entities_.size())
		return nullptr;

	auto &e = entities_[idx];
	// We don't even need to check if e.hasValue(), because if it doesn't,
	// its generation will be 0xffff... and the check will fail

	if (e->generation != generation)
		return nullptr;

	return &e->ent;
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(const Context &ctx, const Entity::PackObject &obj) {
	size_t generation = nextGeneration();
	size_t idx = entities_.emplace(EntWrapper<Ent>{
		generation, std::move(Ent(ctx, obj)) });

	return { this, idx, generation };
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::update(const Context &ctx, float dt) {
	for (auto &ent: entities_)
		ent->ent.update(ctx, dt);
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tick(const Context &ctx, float dt) {
	for (auto &ent: entities_)
		ent->ent.tick(ctx, dt);
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::draw(const Context &ctx, Win &win) {
	for (auto &ent: entities_)
		ent->ent.draw(ctx, win);
}

}
