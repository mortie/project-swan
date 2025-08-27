#include "ResourceManager.h"

#include <cstdio>
#include <cstdlib>
#include <swan/util.h>
#include <string.h>
#include <iostream>

namespace Cygnet {

ResourceBuilder::ResourceBuilder(Renderer *rnd): rnd_(rnd)
{
	fluids_ = std::make_unique<uint8_t[]>(256 * 4 * 2);
}

RenderSprite ResourceBuilder::addSprite(void *data, SpriteMeta meta)
{
	sprites_.push_back(rnd_->createSprite(
		data, meta.width, meta.height, meta.frameHeight, meta.repeatFrom));
	return sprites_.back();
}

RenderMask ResourceBuilder::addMask(void *data, MaskMeta meta)
{
	masks_.push_back(rnd_->createMask(
		data, meta.width, meta.height));
	return masks_.back();
}

void ResourceBuilder::addTileAsset(std::string name, void *data, int frames)
{
	uint16_t startIndex = atlasIndex_;
	for (int i = 0; i < frames; ++i) {
		constexpr size_t size = Swan::TILE_SIZE * Swan::TILE_SIZE * 4;
		unsigned char *ptr = (unsigned char *)data + size * i;
		atlas_.addTile(atlasIndex_, ptr);
		if (frames > 1) {
			tileAssets_[Swan::cat(name, "@", i)] = { startIndex + i, 1 };
		}
		atlasIndex_ += 1;
	}

	tileAssets_[std::move(name)] = { startIndex, uint16_t(frames) };
}

void ResourceBuilder::addTile(Renderer::TileID id, std::string_view assetName)
{
	int assetIndex = 0;
	auto it = tileAssets_.find(assetName);
	if (it == tileAssets_.end()) {
		std::cerr << "Cygnet: Referenced unknown tile asset " << assetName << '\n';
	} else {
		auto &asset = it->second;
		assetIndex = asset.startIndex;
		if (asset.frameCount > 1) {
			tileAnimations_.push_back({
				.tileID = id,
				.startIndex = uint16_t(asset.startIndex),
				.frameCount = uint16_t(asset.frameCount),
				.index = 0,
			});
		}
	}

	while (tiles_.size() <= id) {
		tiles_.push_back(0);
	}
	tiles_[id] = assetIndex;
}

void ResourceBuilder::addFluid(uint8_t id, ByteColor fg, ByteColor bg)
{
	// The two high bits are used for metadata,
	// so register the same color for all combinations
	for (int high = 0; high < 4; ++high) {
		size_t index = (id & 0x3f) | (high << 6);
		auto *pix = &fluids_[index * 4];
		pix[0] = fg.r;
		pix[1] = fg.g;
		pix[2] = fg.b;
		pix[3] = fg.a;
		pix = &fluids_[index * 4 + 256 * 4];
		pix[0] = bg.r;
		pix[1] = bg.g;
		pix[2] = bg.b;
		pix[3] = bg.a;
	}
}

ResourceManager::ResourceManager(ResourceBuilder &&builder):
	rnd_(builder.rnd_), sprites_(std::move(builder.sprites_)),
	tileAnimations_(std::move(builder.tileAnimations_))
{
	size_t width, height;
	const unsigned char *data = builder.atlas_.getImage(&width, &height);

	const char *exportAtlas = getenv("SWAN_EXPORT_ATLAS");
	if (exportAtlas && std::string_view(exportAtlas) == "1") {
		FILE *f = fopen("atlas.rgba", "wb");
		fwrite(data, 1, width * Swan::TILE_SIZE * height * Swan::TILE_SIZE, f);
		fclose(f);
		std::cerr
			<< "Cygnet: Wrote tile atlas to atlas.rgba ("
			<< width << 'x' << height << ")\n";
	}

	rnd_->uploadTileAtlas(data, width, height);
	rnd_->uploadTileMap(builder.tiles_);
	rnd_->uploadFluidAtlas(builder.fluids_.get());
}

ResourceManager::~ResourceManager()
{
	for (auto &sprite: sprites_) {
		rnd_->destroySprite(sprite);
	}

	for (auto &mask: masks_) {
		rnd_->destroyMask(mask);
	}
}

void ResourceManager::tick()
{
	for (auto &anim: tileAnimations_) {
		anim.index = (anim.index + 1) % anim.frameCount;
		rnd_->modifyTile(anim.tileID, uint16_t(anim.startIndex + anim.index));
	}
}

}
