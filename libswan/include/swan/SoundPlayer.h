#pragma once

#include <atomic>
#include <memory>
#include <optional>

#include "assets.h"

namespace Swan {

class SoundPlayer {
public:
	struct Handle {
		std::atomic<bool> done = false;
	};

	struct Context;

	SoundPlayer();
	~SoundPlayer();

	static std::shared_ptr<Handle> makeHandle()
	{
		return std::make_shared<Handle>();
	}

	void volume(float volume);
	float volume();
	void flush();

	void play(
		SoundAsset *asset, float volume,
		std::optional<std::pair<float, float>> center)
	{
		play(asset, volume, center, nullHandle_);
	}

	void play(
		SoundAsset *asset, float volume,
		std::optional<std::pair<float, float>> center,
		std::shared_ptr<Handle> handle);

	void setCenter(float x, float y);

private:
	std::shared_ptr<Handle> nullHandle_;

	void *stream_;
	bool ok_ = false;
	std::unique_ptr<Context> context_;
};

}
