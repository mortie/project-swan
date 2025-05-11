#include "ResourceManager.h"

#include <string.h>

namespace Cygnet {

ResourceBuilder::ResourceBuilder(Renderer *rnd): rnd_(rnd)
{
	fluids_ = std::make_unique<uint8_t[]>(256 * 4);
}

RenderSprite ResourceBuilder::addSprite(
	std::string name, void *data, SpriteMeta meta)
{
	return sprites_[std::move(name)] = rnd_->createSprite(
		data, meta.width, meta.height, meta.frameHeight, meta.repeatFrom);
}

void ResourceBuilder::addTile(uint16_t id, void *data, int frames)
{
	if (frames == 0) {
		atlas_.addTile(id, data);
	}
	else {
		auto ptr = std::make_unique<unsigned char[]>(
			Swan::TILE_SIZE * Swan::TILE_SIZE * 4 * frames);
		memcpy(ptr.get(), data, Swan::TILE_SIZE * Swan::TILE_SIZE * 4 * frames);
		addTile(id, std::move(ptr), frames);
	}
}

void ResourceBuilder::addTile(
	Renderer::TileID id, std::unique_ptr<unsigned char[]> data, int frames)
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

void ResourceBuilder::addFluid(uint8_t id, ByteColor color)
{
	// The two high bits are used for metadata,
	// so register the same color for all combinations
	for (int high = 0; high < 4; ++high) {
		size_t index = (id & 0x3f) | (high << 6);
		auto *pix = &fluids_[index * 4];
		pix[0] = color.r;
		pix[1] = color.g;
		pix[2] = color.b;
		pix[3] = color.a;
	}
}

ResourceManager::ResourceManager(ResourceBuilder &&builder):
	rnd_(builder.rnd_), sprites_(std::move(builder.sprites_)),
	tileAnims_(std::move(builder.tileAnims_))
{
	size_t width, height;
	const unsigned char *data = builder.atlas_.getImage(&width, &height);

	rnd_->uploadTileAtlas(data, width, height);
	rnd_->uploadFluidAtlas(builder.fluids_.get());
}

ResourceManager::~ResourceManager()
{
	for (auto &[name, sprite]: sprites_) {
		rnd_->destroySprite(sprite);
	}
}

void ResourceManager::tick()
{
	// TODO: Maybe do a GPU->GPU copy instead of an upload from the CPU?
	for (auto &anim: tileAnims_) {
		anim.index = (anim.index + 1) % anim.frames;
		unsigned char *data = anim.data.get() +
			Swan::TILE_SIZE * Swan::TILE_SIZE * 4 * anim.index;
		rnd_->modifyTile(anim.id, data);
	}
}

}
