#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <stdint.h>

#include "util.h"
#include "common.h"

namespace Cygnet {
class ResourceBuilder;
}

namespace Swan {

struct ImageAsset {
	int width;
	int frameHeight;
	int frameCount;
	int repeatFrom;
	std::unique_ptr<unsigned char[]> data;
};

struct SoundAsset {
	float *l = nullptr, *r = nullptr;
	std::unique_ptr<float[]> data;
	size_t length = 0;
};

void loadSpriteAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder);

void loadTileOrItemAssets(
	std::string base, std::string path,
	HashMap<ImageAsset> &assets);

void loadSoundAssets(
	std::string base, std::string path,
	HashMap<SoundAsset> &assets);

}
