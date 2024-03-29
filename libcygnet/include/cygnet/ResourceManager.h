#pragma once

#include <unordered_map>
#include <memory>
#include <stdint.h>
#include <string.h>
#include <swan-common/constants.h>

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
	ResourceBuilder(Renderer *rnd): rnd_(rnd)
	{}

	struct SpriteMeta {
		int width;
		int height;
		int frameHeight;
		int repeatFrom;
	};

	RenderSprite addSprite(std::string name, void *data, SpriteMeta meta);
	RenderSprite addSprite(std::string name, void *data);
	void addTile(Renderer::TileID id, void *data, int frames = 1);
	void addTile(Renderer::TileID id, std::unique_ptr<unsigned char[]> data, int frames = 1);

private:
	Renderer *rnd_;
	std::unordered_map<std::string, RenderSprite> sprites_;
	std::vector<ResourceTileAnimation> tileAnims_;
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
	std::unordered_map<std::string, RenderSprite> sprites_;
	std::unordered_map<std::string, Renderer::TileID> tiles_;
	std::vector<ResourceTileAnimation> tileAnims_;
};

inline RenderSprite ResourceBuilder::addSprite(
	std::string name, void *data, SpriteMeta meta)
{
	return sprites_[std::move(name)] = rnd_->createSprite(
		data, meta.width, meta.height, meta.frameHeight, meta.repeatFrom);
}

inline void ResourceBuilder::addTile(uint16_t id, void *data, int frames)
{
	if (frames == 0) {
		atlas_.addTile(id, data);
	}
	else {
		auto ptr = std::make_unique<unsigned char[]>(
			SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE * 4 * frames);
		memcpy(ptr.get(), data, SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE * 4 * frames);
		addTile(id, std::move(ptr), frames);
	}
}

inline void ResourceBuilder::addTile(Renderer::TileID id, std::unique_ptr<unsigned char[]> data, int frames)
{
	atlas_.addTile(id, data.get());
	if (frames > 1) {
		tileAnims_.push_back({
			.id = id,
			.frames = frames,
			.index = 0,
			.data = std::move(data),
		});
	}
}

}
