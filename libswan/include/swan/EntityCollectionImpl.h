#pragma once

#include "EntityCollection.h"
#include "WorldPlane.h"
#include "GameIO.h"
#include <fstream>

#include <capnp/message.h>
#include <capnp/serialize-packed.h>

namespace Swan {

template<typename Ent>
class EntityCollectionImpl final: public EntityCollection {
public:
	struct Wrapper {
		template<typename ... Args>
		Wrapper(Args && ... args): ent(std::forward<Args>(args)...)
		{}

		Wrapper(Wrapper &&other):
			ent(std::move(other.ent)), id(other.id)
		{}
		Wrapper(const Wrapper &) = delete;

		Wrapper &operator=(Wrapper &&other)
		{
			ent = std::move(other.ent);
			id = other.id;
			return *this;
		}

		Wrapper &operator=(const Wrapper &) = delete;

		Ent ent;
		uint64_t id;
	};

	EntityCollectionImpl(std::string name): name_(std::move(name))
	{}

	template<typename ... Args>
	EntityRef spawn(Ctx &ctx, Args && ... args);
	EntityRef spawnMove(Ctx &ctx, Ent &&ent);

	EntityRef spawn(Ctx &ctx) override;
	EntityRef spawn(Ctx &ctx, capnp::Data::Reader data) override;

	size_t size() override
	{
		return entities_.size();
	}

	Entity *get(uint64_t id) override;
	Body *getBody(uint64_t id) override;

	const std::string &name() override
	{
		return name_;
	}

	std::type_index type() override
	{
		return typeid(Ent);
	}

	bool hasUpdated() override
	{
		return (
			entities_.size() > 0 ||
			newEntitiesThisTick_.size() > 0 ||
			despawnedEntitiesThisTick_.size() > 0);
	}

	void update(Ctx &ctx, float dt) override;
	void tick(Ctx &ctx, float dt) override;
	void tick2(Ctx &ctx, float dt) override;
	void tickDone(Ctx &ctx) override;
	void draw(Ctx &ctx, Cygnet::Renderer &rnd) override;
	void erase(Ctx &ctx, uint64_t id) override;
	void onWorldLoaded(Ctx &ctx) override;

	void serialize(
		Ctx &ctx, proto::EntitySystem::Collection::Builder w) override;
	void deserialize(
		Ctx &ctx, proto::EntitySystem::Collection::Reader r) override;

	void serializeUpdates(
		Ctx &ctx, mp_proto::EntityCollectionUpdate::Builder w) override;
	void deserializeUpdates(
		Ctx &ctx, mp_proto::EntityCollectionUpdate::Reader w,
		std::optional<uint64_t> ignoredID) override;

	const std::string name_;
	uint64_t nextId_ = 0;
	std::vector<Wrapper> entities_;
	std::unordered_map<uint64_t, size_t> idToIndex_;
	bool hasTicked_ = false;
	std::vector<uint64_t> newEntitiesThisTick_;
	std::vector<uint64_t> despawnedEntitiesThisTick_;
};

/*
 * EntityRef
 */

template<typename Func>
inline EntityRef &EntityRef::then(Func func)
{
	Entity *ent = coll_->get(id_);

	if (ent != nullptr) {
		func(ent);
	}

	return *this;
}

inline Entity *EntityRef::get()
{
	if (!coll_) {
		return nullptr;
	}

	return coll_->get(id_);
}

inline const Entity *EntityRef::get() const
{
	if (!coll_) {
		return nullptr;
	}

	return coll_->get(id_);
}

inline Body *EntityRef::getBody()
{
	if (!coll_) {
		return nullptr;
	}

	return coll_->getBody(id_);
}

template<typename Trait>
inline auto *EntityRef::trait()
{
	using Tag = typename Trait::Tag;
	auto *t = dynamic_cast<Trait *>(get());
	if (!t) {
		return (decltype(&t->get(Tag{}))) nullptr;
	}

	return &t->get(Tag{});
}

template<typename Trait, typename Func>
inline void EntityRef::traitThen(Func func)
{
	auto *t = trait<Trait>();

	if (t) {
		func(*t);
	}
}

inline bool EntityRef::exists() const
{
	return get() != nullptr;
}


inline std::ostream &operator<<(std::ostream &os, const EntityRef &ref)
{
	if (ref.exists()) {
		os << "(EntityRef " << ref.collection()->name() << " #" << ref.id() << ')';
	} else {
		os << "(EntityRef nil)";
	}
	return os;
}

/*
 * EntityCollection
 */

template<typename Ent, typename ... Args>
inline EntityRef EntityCollection::spawn(Ctx &ctx, Args &&...args)
{
	auto *impl = (EntityCollectionImpl<Ent> *)this;
	return impl->spawn(ctx, std::forward<Args>(args)...);
}

template<typename Ent>
inline EntityRef EntityCollection::spawnMove(Ctx &ctx, Ent &&ent)
{
	auto *impl = (EntityCollectionImpl<Ent> *)this;
	return impl->spawnMove(ctx, std::move(ent));
}

inline EntityRef EntityCollection::currentEntity()
{
	return {this, currentId_};
}

/*
 * EntityCollectionImpl
 */

template<typename Ent>
template<typename ... Args>
inline EntityRef EntityCollectionImpl<Ent>::spawn(Ctx &ctx, Args &&... args)
{
	uint64_t id = nextId_++;
	auto prevCurrentId = currentId_;
	currentId_ = id;

	size_t index = entities_.size();
	auto &w = entities_.emplace_back(ctx, std::forward<Args>(args)...);

	idToIndex_[id] = index;
	w.id = id;
	newEntitiesThisTick_.push_back(id);

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		Body &body = w.ent.get(BodyTrait::Tag{});
		body.pos -= body.size / 2;
		body.chunkPos = chunkPos({tilePos(body.pos)});
		auto &chunk = ctx.plane.getChunk(body.chunkPos);
		chunk.entities_.insert({this, id});
	}

	currentId_ = prevCurrentId;
	return {this, id};
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawnMove(Ctx &ctx, Ent &&ent)
{
	uint64_t id = nextId_++;
	auto prevCurrentId = currentId_;
	currentId_ = id;

	size_t index = entities_.size();
	auto &w = entities_.emplace_back(std::move(ent));

	idToIndex_[id] = index;
	w.id = id;
	newEntitiesThisTick_.push_back(id);

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		Body &body = w.ent.get(BodyTrait::Tag{});
		body.pos -= body.size / 2;
		body.chunkPos = chunkPos({tilePos(body.pos)});
		auto &chunk = ctx.plane.getChunk(body.chunkPos);
		chunk.entities_.insert({this, id});
	}

	currentId_ = prevCurrentId;
	return {this, id};
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(Ctx &ctx)
{
	uint64_t id = nextId_++;
	auto prevCurrentId = currentId_;
	currentId_ = id;

	size_t index = entities_.size();
	auto &w = entities_.emplace_back(ctx);

	w.id = id;
	idToIndex_[id] = index;

	currentId_ = prevCurrentId;
	newEntitiesThisTick_.push_back(id);
	return {this, id};
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(
	Ctx &ctx, capnp::Data::Reader data)
{
	auto ent = spawn(ctx);

	auto prevCurrentId = currentId_;
	currentId_ = ent.id();

	kj::ArrayInputStream stream(data);
	capnp::PackedMessageReader reader(stream);
	try {
		Ent *e = (Ent *)ent.get();
		e->deserialize(ctx, reader);
	} catch (std::exception &ex) {
		warn << "Failed to spawn " << name_ << ": " << ex.what();
		erase(ctx, ent);
		return {};
	}

	currentId_ = prevCurrentId;
	return ent;
}

template<typename Ent>
inline Entity *EntityCollectionImpl<Ent>::get(uint64_t id)
{
	auto indexIt = idToIndex_.find(id);

	if (indexIt == idToIndex_.end()) {
		return nullptr;
	}

	return &entities_[indexIt->second].ent;
}

template<typename Ent>
inline Body *EntityCollectionImpl<Ent>::getBody(uint64_t id)
{
	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		auto indexIt = idToIndex_.find(id);
		if (indexIt == idToIndex_.end()) {
			return nullptr;
		}

		return &entities_[indexIt->second].ent.get(BodyTrait::Tag{});
	}
	else {
		return nullptr;
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::update(Ctx &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("update");
		currentId_ = w.id;
		w.ent.update(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tick(Ctx &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("tick");
		currentId_ = w.id;
		w.ent.tick(ctx, dt);

		if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
			Body &body = w.ent.get(BodyTrait::Tag{});
			auto newChunkPos = chunkPos(tilePos(body.pos));
			if (hasTicked_ && newChunkPos == body.chunkPos) {
				continue;
			}

			EntityRef ref{this, w.id};
			ctx.plane.getChunk(body.chunkPos).entities_.erase(ref);
			ctx.plane.getChunk(newChunkPos).entities_.insert(ref);
			body.chunkPos = newChunkPos;
		}
	}

	hasTicked_ = true;
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tick2(Ctx &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("tick2");
		currentId_ = w.id;
		w.ent.tick2(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tickDone(Ctx &ctx)
{
	newEntitiesThisTick_.clear();
	despawnedEntitiesThisTick_.clear();
}


template<typename Ent>
inline void EntityCollectionImpl<Ent>::draw(Ctx &ctx, Cygnet::Renderer &rnd)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("draw");
		w.ent.draw(ctx, rnd);
	}

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		if (ctx.game.debug_.drawCollisionBoxes) {
			for (auto &w: entities_) {
				auto &body = w.ent.get(BodyTrait::Tag{});
				rnd.drawRect({body.pos, body.size});
			}
		}
	}

	if (!rnd.assertUIViewStackEmpty()) {
		warn << "UI view stack is not empty after drawing " << name() << '!';
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::erase(Ctx &ctx, uint64_t id)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	auto indexIt = idToIndex_.find(id);
	if (indexIt == idToIndex_.end()) {
		Swan::warn
			<< "Attempt to delete non-existent '" << typeid(Ent).name()
			<< "' entity with ID " << id;
		return;
	}

	despawnedEntitiesThisTick_.push_back(id);
	size_t index = indexIt->second;

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		auto &w = entities_[index];
		Body &body = w.ent.get(BodyTrait::Tag{});
		ctx.plane.getChunk(body.chunkPos).entities_.erase({this, w.id});
	}

	if (index == entities_.size() - 1) {
		entities_.pop_back();
		idToIndex_.erase(id);
		return;
	}

	entities_[index] = std::move(entities_.back());
	entities_.pop_back();
	idToIndex_.erase(id);
	idToIndex_[entities_[index].id] = index;
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::onWorldLoaded(Ctx &ctx)
{
	for (auto &w: entities_) {
		w.ent.onWorldLoaded(ctx);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::serialize(
	Ctx &ctx, proto::EntitySystem::Collection::Builder w)
{
	std::string sanitizedName;
	if (ctx.game.debug_.outputEntityProto) {
		sanitizedName = name_;
		for (auto &ch: sanitizedName) {
			if (ch == ':') {
				ch = '_';
			}
		}
	}

	// TODO: Do this more intelligently somehow
	auto scratch = kj::heapArray<capnp::word>(1024);
	auto scratchBytes = scratch.asBytes();
	memset(&scratchBytes.front(), 0, scratchBytes.size());
	kj::VectorOutputStream stream;

	w.setName(name_);
	w.setNextID(nextId_);
	auto entities = w.initEntities(entities_.size());
	for (size_t i = 0; i < entities_.size(); ++i) {
		auto &wrapper = entities_[i];
		entities[i].setId(wrapper.id);

		capnp::MallocMessageBuilder mb;
		wrapper.ent.serialize(ctx, mb);

		stream.clear();
		capnp::writePackedMessage(stream, mb);

		auto arr = stream.getArray();
		auto data = entities[i].initData(arr.size());
		memcpy(&data.front(), &arr.front(), arr.size());

		if (ctx.game.debug_.outputEntityProto) {
			auto path = cat("ent.", sanitizedName, ".", wrapper.id, ".bin");
			std::ofstream f(path);
			if (f) {
				info << "Writing entity to " << path << "...";
				f.write((char *)&arr.front(), arr.size());
			} else {
				warn << "Failed to open " << path << '!';
			}
		}
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::deserialize(
	Ctx &ctx, proto::EntitySystem::Collection::Reader r)
{
	entities_.clear();
	idToIndex_.clear();
	nextId_ = r.getNextID();
	hasTicked_ = false;

	entities_.reserve(r.getEntities().size());
	for (auto entity: r.getEntities()) {
		size_t index = entities_.size();
		currentId_ = entity.getId();
		auto &wrapper = entities_.emplace_back(ctx);
		wrapper.id = entity.getId();

		auto data = entity.getData();
		kj::ArrayInputStream stream(data);
		capnp::PackedMessageReader reader(stream);
		try {
			wrapper.ent.deserialize(ctx, reader);
			idToIndex_[wrapper.id] = index;
		} catch (std::exception &ex) {
			warn << "Failed to deserialize " << name_ << " entity: " << ex.what();
			entities_.pop_back();
		}
	}
}

template<typename Ent>
void EntityCollectionImpl<Ent>::serializeUpdates(
	Ctx &ctx, mp_proto::EntityCollectionUpdate::Builder w)
{
	// TODO: Do this more intelligently somehow
	auto scratch = kj::heapArray<capnp::word>(1024);
	auto scratchBytes = scratch.asBytes();
	memset(&scratchBytes.front(), 0, scratchBytes.size());
	kj::VectorOutputStream stream;

	auto newEntities = w.initNewEntities(newEntitiesThisTick_.size());
	for (size_t i = 0; auto id: newEntitiesThisTick_) {
		newEntities.set(i++, id);
	}

	auto despawnedEntities = w.initDespawnedEntities(despawnedEntitiesThisTick_.size());
	for (size_t i = 0; auto id: despawnedEntitiesThisTick_) {
		despawnedEntities.set(i++, id);
	}

	std::vector<size_t> updatedIndexes;
	for (size_t i = 0; i < entities_.size(); ++i) {
		if (entities_[i].ent.hasUpdated()) {
			updatedIndexes.push_back(i);
		}
	}

	auto entities = w.initUpdatedEntities(updatedIndexes.size());
	for (size_t i = 0; size_t index: updatedIndexes) {
		auto &wrapper = entities_[index];
		auto entity = entities[i++];
		entity.setId(wrapper.id);

		capnp::MallocMessageBuilder mb;
		wrapper.ent.serializeUpdates(ctx, mb);

		stream.clear();
		capnp::writePackedMessage(stream, mb);

		auto arr = stream.getArray();
		auto data = entity.initData(arr.size());
		memcpy(&data.front(), &arr.front(), arr.size());
	}
}

template<typename Ent>
void EntityCollectionImpl<Ent>::deserializeUpdates(
	Ctx &ctx, mp_proto::EntityCollectionUpdate::Reader r,
	std::optional<uint64_t> ignoredID)
{
	// Despawn despawned entities
	for (auto id: r.getDespawnedEntities()) {
		erase(ctx, id);
	}

	// Spawn new entities
	for (auto id: r.getNewEntities()) {
		if (idToIndex_.contains(id)) {
			warn << "Was told that ID " << id << " just spawned, but it already exists!";
			continue;
		}

		size_t index = entities_.size();
		auto &w = entities_.emplace_back(ctx);
		w.id = id;
		idToIndex_[id] = index;
	}

	// Deserialize updated entities
	for (auto entity: r.getUpdatedEntities()) {
		uint64_t id = entity.getId();
		if (ignoredID && *ignoredID == id) {
			continue;
		}

		auto it = idToIndex_.find(id);
		if (it == idToIndex_.end()) {
			warn << "Update for non-existent entity with ID " << id;
			continue;
		}

		auto &wrapper = entities_[it->second];

		// This is gonna need some updates for netcode optimization too
		auto data = entity.getData();
		kj::ArrayInputStream stream(data);
		capnp::PackedMessageReader reader(stream);
		currentId_ = id;
		try {
			wrapper.ent.deserializeUpdates(ctx, reader);
		} catch (std::exception &ex) {
			warn << "Failed to deserialize " << name_ << " entity: " << ex.what();
		}
	}
}

}
