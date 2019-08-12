#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include <SFML/Graphics/Image.hpp>

#include "Tile.h"
#include "WorldGen.h"
#include "Entity.h"
#include "Asset.h"

namespace Swan {

class Mod {
public:
	using ModID = uint32_t;

	void init(const std::string &name);
	void registerTile(const std::string &name, Tile *tile);
	void registerWorldGen(const std::string &name, WorldGen::Factory *gen);
	void registerEntity(const std::string &name, Entity::Factory *ent);
	void registerAsset(const std::string &name, Asset *asset);

	std::unique_ptr<sf::Image> loadImage(const std::string &path);

	std::string name_;
	std::string path_;
	std::vector<std::shared_ptr<Tile>> tiles_;
	std::vector<std::shared_ptr<WorldGen::Factory>> worldgens_;
	std::vector<std::shared_ptr<Entity::Factory>> entities_;
	std::vector<std::shared_ptr<Asset>> assets_;
	bool inited_ = false;
};

}
