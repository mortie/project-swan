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

struct SoundAsset {
	float *l = nullptr, *r = nullptr;
	std::unique_ptr<float[]> data;
	size_t length = 0;
};

void loadSpriteAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder);

void loadTileAssets(
	std::string base, std::string path,
	Cygnet::ResourceBuilder &builder);

void loadSoundAssets(
	std::string base, std::string path,
	HashMap<SoundAsset> &assets);

}
