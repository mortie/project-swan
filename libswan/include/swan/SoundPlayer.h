#pragma once

#include <atomic>
#include <memory>

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

	void play(SoundAsset *asset, float volume)
	{
		play(asset, volume, nullHandle_);
	}

	void play(SoundAsset *asset, float volume, std::shared_ptr<Handle> handle);

private:
	std::shared_ptr<Handle> nullHandle_;

	void *stream_;
	bool ok_ = false;
	std::unique_ptr<Context> context_;
};

}
