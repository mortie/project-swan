#pragma once

#include "EntityCollection.h"
#include "WorldPlane.h"
#include "Game.h"

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
	EntityRef spawn(const Context &ctx, Args && ... args);

	EntityRef spawn(const Context &ctx) override;
	EntityRef spawn(const Context &ctx, sbon::ObjectReader r) override;

	size_t size() override
	{
		return entities_.size();
	}

	Entity *get(uint64_t id) override;
	BodyTrait::Body *getBody(uint64_t id) override;

	const std::string &name() override
	{
		return name_;
	}

	std::type_index type() override
	{
		return typeid(Ent);
	}

	void update(const Context &ctx, float dt) override;
	void tick(const Context &ctx, float dt) override;
	void tick2(const Context &ctx, float dt) override;
	void draw(const Context &ctx, Cygnet::Renderer &rnd) override;
	void erase(const Context &ctx, uint64_t id) override;

	void serialize(const Context &ctx, sbon::Writer w) override;
	void deserialize(const Context &ctx, sbon::Reader r) override;

	const std::string name_;
	uint64_t nextId_ = 0;
	std::vector<Wrapper> entities_;
	std::unordered_map<uint64_t, size_t> idToIndex_;
	bool hasTicked_ = false;
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

inline BodyTrait::Body *EntityRef::getBody()
{
	if (!coll_) {
		return nullptr;
	}

	return coll_->getBody(id_);
}

template<typename Trait>
inline auto *EntityRef::trait()
{
	using Tag = Trait::Tag;
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

inline bool EntityRef::exists()
{
	return get() != nullptr;
}

/*
 * EntityCollection
 */

template<typename Ent, typename ... Args>
inline EntityRef EntityCollection::spawn(const Context &ctx, Args &&... args)
{
	auto *impl = (EntityCollectionImpl<Ent> *)this;

	return impl->spawn(ctx, std::forward<Args>(args)...);
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
inline EntityRef EntityCollectionImpl<Ent>::spawn(const Context &ctx, Args &&... args)
{
	uint64_t id = nextId_++;
	size_t index = entities_.size();
	auto &w = entities_.emplace_back(ctx, std::forward<Args>(args)...);

	idToIndex_[id] = index;
	w.id = id;

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		BodyTrait::Body &body = w.ent.get(BodyTrait::Tag{});
		body.pos -= body.size / 2;
		body.chunkPos = tilePosToChunkPos({(int)body.pos.x, (int)body.pos.y});
		auto &chunk = ctx.plane.getChunk(body.chunkPos);
		chunk.entities_.insert({this, id});
	}

	return {this, id};
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(const Context &ctx)
{
	uint64_t id = nextId_++;
	size_t index = entities_.size();
	auto &w = entities_.emplace_back(ctx);

	w.id = id;
	idToIndex_[id] = index;

	return {this, id};
}

template<typename Ent>
inline EntityRef EntityCollectionImpl<Ent>::spawn(
	const Context &ctx, sbon::ObjectReader r)
{
	auto ent = spawn(ctx);

	try {
		ent->deserialize(ctx, r);
	} catch (std::exception &ex) {
		warn << "Failed to spawn " << name_ << " from SBON: " << ex.what();
		erase(ctx, ent);
		return {};
	}

	return ent;
}

template<typename Ent>
inline Entity *EntityCollectionImpl<Ent>::get(uint64_t id)
{
	auto indexIt = idToIndex_.find(id);

	if (indexIt == idToIndex_.end()) {
		Swan::info
			<< "Looked for non-existent '" << typeid(Ent).name()
			<< "' entity with ID " << id;
		return nullptr;
	}

	return &entities_[indexIt->second].ent;
}

template<typename Ent>
inline BodyTrait::Body *EntityCollectionImpl<Ent>::getBody(uint64_t id)
{
	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		auto indexIt = idToIndex_.find(id);
		if (indexIt == idToIndex_.end()) {
			Swan::info
				<< "Looked for non-existent '" << typeid(Ent).name()
				<< "' entity with ID " << id;
			return nullptr;
		}

		return &entities_[indexIt->second].ent.get(BodyTrait::Tag{});
	}
	else {
		return nullptr;
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::update(const Context &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("update");
		currentId_ = w.id;
		w.ent.update(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::tick(const Context &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("tick");
		currentId_ = w.id;
		w.ent.tick(ctx, dt);

		if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
			BodyTrait::Body &body = w.ent.get(BodyTrait::Tag{});
			auto newChunkPos = tilePosToChunkPos({(int)body.pos.x, (int)body.pos.y});
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
inline void EntityCollectionImpl<Ent>::tick2(const Context &ctx, float dt)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("tick2");
		currentId_ = w.id;
		w.ent.tick2(ctx, dt);
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::draw(const Context &ctx, Cygnet::Renderer &rnd)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	for (auto &w: entities_) {
		ZoneScopedN("draw");
		w.ent.draw(ctx, rnd);
	}

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		if (ctx.game.debugDrawCollisionBoxes_) {
			for (auto &w: entities_) {
				auto &body = w.ent.get(BodyTrait::Tag{});
				rnd.drawRect({body.pos, body.size});
			}
		}
	}
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::erase(const Context &ctx, uint64_t id)
{
	ZoneScopedN(__PRETTY_FUNCTION__);
	auto indexIt = idToIndex_.find(id);
	if (indexIt == idToIndex_.end()) {
		Swan::warn
			<< "Attempt to delete non-existent '" << typeid(Ent).name()
			<< "' entity with ID " << id;
		return;
	}

	size_t index = indexIt->second;

	if constexpr (std::is_base_of_v<BodyTrait, Ent> ) {
		auto &w = entities_[index];
		BodyTrait::Body &body = w.ent.get(BodyTrait::Tag{});
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
inline void EntityCollectionImpl<Ent>::serialize(
	const Context &ctx, sbon::Writer w)
{
	w.writeObject([&](sbon::ObjectWriter w) {
		w.key("next").writeUInt(nextId_);
		w.key("entities").writeArray([&](sbon::Writer w) {
			for (auto &wrapper: entities_) {
				try {
					w.writeObject([&](sbon::ObjectWriter w) {
						w.key("$id").writeUInt(wrapper.id);
						wrapper.ent.serialize(ctx, w);
					});
				} catch (std::exception &ex) {
					warn << "Failed to serialize " << name_ << " entity: " << ex.what();
				}
			}
		});
	});
}

template<typename Ent>
inline void EntityCollectionImpl<Ent>::deserialize(
	const Context &ctx, sbon::Reader r)
{
	entities_.clear();
	idToIndex_.clear();
	nextId_ = 0;
	hasTicked_ = false;

	auto deserializeEntity = [&](sbon::ObjectReader r) {
		std::optional<uint64_t> id;
		std::string key;
		while (true) {
			auto val = r.next(key);
			if (key == "$id") {
				id = val.getUInt();
				break;
			}
			else {
				warn
					<< "Skipping unknown key '" << key
					<< "' while deserializing " << name_;
			}
		}

		if (!id) {
			warn << "Failed to deserialize " << name_ << " entity: Missing $id";
			return;
		}

		size_t index = entities_.size();
		auto &w = entities_.emplace_back(ctx);
		w.id = id.value();
		try {
			w.ent.deserialize(ctx, r);
			idToIndex_[id.value()] = index;
		} catch (std::exception &ex) {
			warn << "Failed to deserialize " << name_ << " entity: " << ex.what();
			entities_.pop_back();
		}
	};

	r.readObject([&](std::string &key, sbon::Reader val) {
		if (key == "next") {
			nextId_ = val.getUInt();
		}
		else if (key == "entities") {
			val.readArray([&](sbon::Reader val) {
				val.getObject(deserializeEntity);
			});
		}
		else {
			val.skip();
		}
	});
}

}
