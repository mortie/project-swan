#include "ResourceManager.h"

namespace Cygnet {

ResourceManager::ResourceManager(ResourceBuilder &&builder):
	rnd_(builder.rnd_), sprites_(std::move(builder.sprites_)),
	tileAnims_(std::move(builder.tileAnims_))
{
	size_t width, height;
	const unsigned char *data = builder.atlas_.getImage(&width, &height);

	rnd_.uploadTileAtlas(data, width, height);
}

ResourceManager::~ResourceManager()
{
	for (auto &[name, sprite]: sprites_) {
		rnd_.destroySprite(sprite);
	}
}

void ResourceManager::tick()
{
	// TODO: Maybe do a GPU->GPU copy instead of an upload from the CPU?
	for (auto &anim: tileAnims_) {
		anim.index = (anim.index + 1) % anim.frames;
		unsigned char *data = anim.data.get() +
			SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE * 4 * anim.index;
		rnd_.modifyTile(anim.id, data);
	}
}

}
