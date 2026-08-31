#pragma once

#include "../common.h"
#include "../Entity.h"
#include "../EntityCollection.h"
#include "../traits/BodyTrait.h"
#include "swan.capnp.h"
#include <swan/log.h>

#include <memory>
#include <span>
#include <vector>

namespace Cygnet {
class Renderer;
}

namespace Swan {

class WorldPlane;
class TileSystemImpl;

struct FoundEntity {
	EntityRef ref;
	Body &body;
};

class EntitySystemImpl {
public:
	class CurrentEntityOverride {
	public:
		CurrentEntityOverride(EntitySystemImpl *sys, EntityRef ref): sys_(sys)
		{
			sys_->currentEntityStack_.push_back(ref);
		}

		~CurrentEntityOverride()
		{
			sys_->currentEntityStack_.pop_back();
		}

	private:
		EntitySystemImpl *sys_;
	};

	EntitySystemImpl(
		WorldPlane &plane,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	/*
	 * Available to game logic
	 */

	EntityRef spawn(std::string_view name, capnp::Data::Reader data);

	template<typename Ent, typename ...Args>
	EntityRef spawn(Args &&...args)
	{
		auto it = collectionsByType_.find(typeid(Ent));
		if (it == collectionsByType_.end()) {
			warn << "Attempt to spawn unregistered entity: " << typeid(Ent).name();
			return {};
		}

		auto ctx = getContext();
		auto coll = it->second;
		auto *prevCurrentColl = currentCollection_;
		currentCollection_ = coll;
		auto ref = it->second->spawn<Ent, Args...>(ctx, std::forward<Args>(args)...);
		ref->onSpawn(ctx);
		currentCollection_ = prevCurrentColl;
		return ref;
	}

	template<typename Ent>
	EntityRef spawnMove(Ent &&ent)
	{
		auto it = collectionsByType_.find(typeid(Ent));
		if (it == collectionsByType_.end()) {
			warn << "Attempt to spawn unregistered entity: " << typeid(Ent).name();
			return {};
		}

		auto ctx = getContext();
		auto coll = it->second;
		auto *prevCurrentColl = currentCollection_;
		currentCollection_ = coll;
		auto ref = it->second->spawnMove(ctx, std::move(ent));
		ref->onSpawn(ctx);
		currentCollection_ = prevCurrentColl;
		return ref;
	}

	void despawn(EntityRef ref);

	std::span<FoundEntity> getColliding(Body &body);
	std::span<FoundEntity> getInTile(TilePos pos);
	std::span<FoundEntity> getInArea(Vec2 pos, Vec2 size);

	std::span<std::unique_ptr<EntityCollection>> collections()
	{ return collections_; }

	EntityRef getTileEntity(TilePos pos);

	EntityRef current();

	CurrentEntityOverride overrideCurrentEntity(EntityRef ref)
	{ return {this, ref}; }

	/*
	 * Available to friends
	 */

	void spawnTileEntity(TilePos pos, std::string_view name);
	void despawnTileEntity(TilePos pos);

	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	void tick(float dt);
	void tickDone();

	EntityCollection *getCollectionOf(std::string_view name);

	void despawnAllTileEntities();

	void serialize(proto::EntitySystem::Builder w);
	void deserialize(proto::EntitySystem::Reader r);

private:
	Context getContext();

	WorldPlane &plane_;

	std::vector<FoundEntity> foundEntitiesBuf_;

	std::vector<std::unique_ptr<EntityCollection>> collections_;
	std::unordered_map<std::type_index, EntityCollection *> collectionsByType_;
	HashMap<EntityCollection *> collectionsByName_;
	EntityCollection *currentCollection_ = nullptr;
	std::vector<EntityRef> currentEntityStack_;

	std::unordered_map<TilePos, EntityRef> tileEntities_;

	std::vector<EntityRef> despawnListA_;
	std::vector<EntityRef> despawnListB_;

	friend CurrentEntityOverride;
};

class EntitySystem: private EntitySystemImpl {
public:
	using EntitySystemImpl::EntitySystemImpl;

	using EntitySystemImpl::spawn;
	using EntitySystemImpl::spawnMove;
	using EntitySystemImpl::despawn;
	using EntitySystemImpl::getColliding;
	using EntitySystemImpl::getInTile;
	using EntitySystemImpl::getInArea;
	using EntitySystemImpl::getTileEntity;
	using EntitySystemImpl::collections;
	using EntitySystemImpl::current;
	using EntitySystemImpl::overrideCurrentEntity;

	friend WorldPlane;
	friend TileSystemImpl;
	friend EntityRef;
};

}
