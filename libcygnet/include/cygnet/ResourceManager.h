#pragma once

#include <memory>
#include <stdint.h>
#include <swan/constants.h>
#include <swan/HashMap.h>

#include "Renderer.h"
#include "TileAtlas.h"

namespace Cygnet {

struct ResourceTileAnimation {
	Renderer::TileID tileID;
	uint16_t startIndex;
	uint16_t frameCount;
	uint16_t index;
};

class ResourceManager;

class ResourceBuilder {
public:
	ResourceBuilder(Renderer *rnd);

	struct SpriteMeta {
		int width;
		int height;
		int frameHeight;
		int repeatFrom;
	};

	bool hasSprite(std::string_view name) { return sprites_.contains(name); }
	RenderSprite addSprite(std::string name, void *data, SpriteMeta meta);

	bool hasTileAsset(std::string_view name) { return tileAssets_.contains(name); }
	void addTileAsset(std::string name, void *data, int frames);

	void addTile(Renderer::TileID id, std::string_view assetName);
	void addFluid(uint8_t id, ByteColor fg, ByteColor bg);

private:
	struct TileMeta {
		int startIndex;
		int frameCount;
	};

	Renderer *rnd_;
	Swan::HashMap<RenderSprite> sprites_;
	Swan::HashMap<TileMeta> tileAssets_;
	std::vector<uint16_t> tiles_;
	std::vector<ResourceTileAnimation> tileAnimations_;

	std::unique_ptr<uint8_t[]> fluids_;
	TileAtlas atlas_;
	uint16_t atlasIndex_ = 0;

	friend ResourceManager;
};

class ResourceManager {
public:
	ResourceManager() = default;
	ResourceManager(ResourceBuilder &&);
	ResourceManager(ResourceManager &&) = default;
	~ResourceManager();

	ResourceManager &operator=(ResourceManager &&) = default;

	void tick();

	Renderer *rnd_ = nullptr;
	Swan::HashMap<RenderSprite> sprites_;
	Swan::HashMap<Renderer::TileID> tiles_;
	std::vector<ResourceTileAnimation> tileAnimations_;
};

}
