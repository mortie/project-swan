#pragma once

#include <memory>
#include <string>
#include <stdint.h>

#include <cygnet/util.h>
#include <swan/util.h>
#include "common.h"
#include "cygnet/Renderer.h"

namespace Cygnet {
class ResourceBuilder;
}

namespace Swan {

struct TileParticles {
	Cygnet::ByteColor particles[8][8];
};

struct TileAssetMeta {
	float yOffset = 0;
	std::shared_ptr<TileParticles> particles;
};

struct SoundAsset {
	float *l = nullptr, *r = nullptr;
	std::unique_ptr<float[]> data;
	size_t length = 0;
};

void loadSpriteAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder, HashMap<Cygnet::RenderSprite> &sprites);

void loadMaskAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder, HashMap<Cygnet::RenderMask> &masks);

void loadTileAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder, HashMap<TileAssetMeta> &meta);

void loadSoundAssets(
	std::string base, std::string path,
	HashMap<SoundAsset> &assets);

}
