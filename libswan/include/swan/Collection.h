#pragma once

#include <vector>
#include <string>
#include <typeindex>
#include <type_traits>
#include <unordered_map>
#include <stdint.h>

#include "common.h"
#include "log.h"
#include "Entity.h"
#include "util.h"

namespace Swan {

class EntityCollection;

class EntityRef {
public:
	EntityRef() = default;
	EntityRef(EntityCollection *coll, uint64_t id):
		coll_(coll), id_(id) {}

	operator bool() { return hasValue(); }

	template<typename Func>
	EntityRef &then(Func func);

	bool hasValue();
	Entity *get();

private:
	EntityCollection *coll_;
	uint64_t id_;
};

class EntityCollection {
public:
	struct Factory {
		const std::string name;
		std::unique_ptr<EntityCollection> (*const create)(std::string name);
	};

	virtual ~EntityCollection() = default;

	template<typename Ent, typename... Args>
	EntityRef spawn(Args&&... args);

	virtual const std::string &name() = 0;
	virtual std::type_index type() = 0;

	virtual size_t size() = 0;
	virtual Entity *get(uint64_t id) = 0;

	virtual EntityRef spawn(const Context &ctx, const Entity::PackObject &obj) = 0;
	virtual void update(const Context &ctx, float dt) = 0;
	virtual void tick(const Context &ctx, float dt) = 0;
	virtual void draw(const Context &ctx, Cygnet::Renderer &rnd) = 0;
	virtual void erase(uint64_t id) = 0;
};

template<typename Ent>
class EntityCollectionImpl final: public EntityCollection {
public:
	EntityCollectionImpl(std::string name): name_(std::move(name)) {}

	template<typename... Args>
	EntityRef spawn(Args&&... args);

	size_t size() override { return entities_.size(); }
	Entity *get(uint64_t id) override;

	const std::string &name() override { return name_; }
	std::type_index type() override { return typeid(Ent); }

	EntityRef spawn(const Context &ctx, const Entity::PackObject &obj) override;
	void update(const Context &ctx, float dt) override;
	void tick(const Context &ctx, float dt) override;
	void draw(const Context &ctx, Cygnet::Renderer &rnd) override;
	void erase(uint64_t id) override;

	const std::string name_;
	uint64_t nextId_ = 0;
	std::vector<Ent> entities_;
	std::unordered_map<uint64_t, size_t> idToIndex_;
};

/*
 * EntityRef
 */

template<typename Func>
inline EntityRef &EntityRef::then(Func func) {
	Entity *ent = coll_->get(id_);
	if (ent != nullptr)
		func(ent);

	return *this;
}

inline Entity *EntityRef::get() {
	return coll_->get(id_);
}

inline bool EntityRef::hasValue() {
	return get() != nullptr;
}

/*
 * EntityCollection
 */


template<typename Ent, typename... Args>
inline EntityRef EntityCollection::spawn(Args&&... args) {
	auto *impl = (EntityCollectionImpl<Ent> *)this;
	return impl->spawn(std::forward<Args>(args)...);
}

/*
 * EntityCollectionImpl
 */

template<typename Ent>
template<typename... Args>
inline EntityRef EntityCollectionImpl<Ent>::spawn(Args&&... args) {
	uint64_t id = nextId_++;
	size_t index = entities_.size();
	entities_.emplace_back(std::forward<Args>(args)...);
	entities_.back().id_ = id;
	idToIndex_[id] = index;
	return {this, id};
}

template<typename Ent>
inline Entity *EntityCollectionImpl<Ent>::get(uint64_t id) {
	auto indexIt = idToIndex_.find(id);
	if (indexIt == idToIndex_.end()) {
		Swan::info
			<< "Looked for non-existent '" << typeid(Ent).name()
			<< "' entity with ID " << id;
		return nullptr;
	}

	return &entities_[indexIt->second];
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(const Context &ctx, const Entity::PackObject &obj) {
	return spawn(ctx, obj);
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::update(const Context &ctx, float dt) {
	ZoneScopedN(typeid(Ent).name());
	for (auto &ent: entities_) {
		ZoneScopedN("update");
		ent.update(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tick(const Context &ctx, float dt) {
	ZoneScopedN(typeid(Ent).name());
	for (auto &ent: entities_) {
		ZoneScopedN("tick");
		ent.tick(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::draw(const Context &ctx, Cygnet::Renderer &rnd) {
	ZoneScopedN(typeid(Ent).name());
	for (auto &ent: entities_) {
		ZoneScopedN("draw");
		ent.draw(ctx, rnd);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::erase(uint64_t id) {
	ZoneScopedN(typeid(Ent).name());
	auto indexIt = idToIndex_.find(id);
	if (indexIt == idToIndex_.end()) {
		Swan::warn
			<< "Attempt to delete non-existent '" << typeid(Ent).name()
			<< "' entity with ID " << id;
		return;
	}

	size_t index = indexIt->second;
	if (index == entities_.size() - 1) {
		entities_.pop_back();
		return;
	}

	entities_[index] = std::move(entities_.back());
	entities_.pop_back();
	idToIndex_.erase(id);
	idToIndex_[entities_[index].id_] = index;
}

}
