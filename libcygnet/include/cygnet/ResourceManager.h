#pragma once

#include <unordered_map>
#include <memory>
#include <stdint.h>
#include <string.h>
#include <swan/constants.h>
#include <swan/HashMap.h>

#include "Renderer.h"
#include "TileAtlas.h"

namespace Cygnet {

struct ResourceTileAnimation {
	uint16_t id;
	int frames;
	int index;
	std::unique_ptr<unsigned char[]> data;
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

	RenderSprite addSprite(std::string name, void *data, SpriteMeta meta);
	void addTile(Renderer::TileID id, void *data, int frames = 1);
	void addTile(Renderer::TileID id, std::unique_ptr<unsigned char[]> data, int frames = 1);
	void addFluid(uint8_t id, ByteColor color);

private:
	Renderer *rnd_;
	Swan::HashMap<RenderSprite> sprites_;
	std::vector<ResourceTileAnimation> tileAnims_;
	std::unique_ptr<uint8_t[]> fluids_;
	TileAtlas atlas_;

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
	std::vector<ResourceTileAnimation> tileAnims_;
};

}
