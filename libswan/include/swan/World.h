#pragma once

#include <memory>
#include <vector>
#include <string>

#include "common.h"
#include "Asset.h"
#include "Item.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "WorldGen.h"
#include "Entity.h"

namespace Swan {

class Game;

class World {
public:
	WorldPlane &addPlane(std::string gen);
	WorldPlane &addPlane() { return addPlane(default_world_gen_); }
	void setCurrentPlane(WorldPlane &plane);
	void setWorldGen(const std::string &gen);
	void spawnPlayer();
	void registerTile(std::shared_ptr<Tile> t);
	void registerWorldGen(std::shared_ptr<WorldGen::Factory> gen);
	void registerEntity(std::shared_ptr<Entity::Factory> ent);
	void registerAsset(std::shared_ptr<Asset> asset);

	Asset &getAsset(const std::string &name);
	Item &getItem(const std::string &name);
	Tile::ID getTileID(const std::string &name);
	Tile &getTileByID(Tile::ID id);
	Tile &getTile(const std::string &name);

	void draw(Game &game, Win &win);
	void update(Game &game, float dt);
	void tick();

	std::map<std::string, std::shared_ptr<WorldGen::Factory>> worldgens_;
	std::map<std::string, std::shared_ptr<Entity::Factory>> ents_;
	std::map<std::string, std::shared_ptr<Asset>> assets_;
	std::map<std::string, std::shared_ptr<Item>> items_;
	std::vector<std::shared_ptr<Tile>> tiles_;
	std::map<std::string, Tile::ID> tiles_map_;
	Entity *player_;

private:
	class ChunkRenderer {
	public:
		void tick(WorldPlane &plane, ChunkPos abspos);
	};

	ChunkRenderer chunk_renderer_;
	WorldPlane::ID current_plane_;
	std::vector<WorldPlane> planes_;
	std::string default_world_gen_;
};

}
