#pragma once

#include "../common.h"
#include "../Entity.h"
#include "../EntityCollection.h"
#include "../traits/BodyTrait.h"
#include "../log.h"

#include <memory>
#include <span>
#include <vector>

namespace Cygnet {
class Renderer;
}

namespace Swan {

class WorldPlane;

class EntitySystem {
public:
	struct FoundEntity {
		EntityRef ref;
		BodyTrait::Body &body;
	};

	EntitySystem(
		WorldPlane &plane,
		std::vector<std::unique_ptr<EntityCollection>> &&colls);

	EntityRef spawn(std::string_view name, sbon::ObjectReader r);

	template<typename Ent, typename ...Args>
	EntityRef spawn(Args &&...args);

	void despawn(EntityRef ref);

	std::span<FoundEntity> getColliding(BodyTrait::Body &body);
	std::span<FoundEntity> getInTile(TilePos pos);
	std::span<FoundEntity> getInArea(Vec2 pos, Vec2 size);

	EntityRef getTileEntity(TilePos pos);

	void spawnTileEntity(TilePos pos, std::string_view name);
	void despawnTileEntity(TilePos pos);

	EntityRef current();

private:
	Context getContext();

	void draw(Cygnet::Renderer &rnd);
	void update(float dt);
	void tick(float dt);

	EntityCollection *getCollectionOf(std::string_view name);

	void serialize(sbon::Writer w);
	void deserialize(sbon::Reader r);

	WorldPlane &plane_;

	std::vector<FoundEntity> foundEntitiesBuf_;

	std::vector<std::unique_ptr<EntityCollection>> collections_;
	std::unordered_map<std::type_index, EntityCollection *> collectionsByType_;
	HashMap<EntityCollection *> collectionsByName_;
	EntityCollection *currentCollection_ = nullptr;

	std::unordered_map<TilePos, EntityRef> tileEntities_;

	std::vector<EntityRef> despawnListA_;
	std::vector<EntityRef> despawnListB_;

	friend WorldPlane;
	friend EntityRef;
};

template<typename Ent, typename ...Args>
inline EntityRef EntitySystem::spawn(Args &&...args)
{
	auto it = collectionsByType_.find(typeid(Ent));
	if (it == collectionsByType_.end()) {
		warn << "Attempt to spawn unregistered entity: " << typeid(Ent).name();
		return {};
	}

	auto ctx = getContext();
	auto ent = it->second->spawn<Ent, Args...>(ctx, std::forward<Args>(args)...);
	ent->onSpawn(ctx);
	return ent;
}

}
