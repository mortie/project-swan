#include "ResourceManager.h"

#include <iostream>

namespace Cygnet {

ResourceBuilder::ResourceBuilder(Renderer *rnd): rnd_(rnd)
{
	fluids_ = std::make_unique<uint8_t[]>(256 * 4);
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
			SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE * 4 * anim.index;
		rnd_->modifyTile(anim.id, data);
	}
}

}
