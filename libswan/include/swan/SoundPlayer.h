#pragma once

#include <memory>
#include <optional>

#include "assets.h"
#include "swan/Vector2.h"

namespace Swan {

class SoundPlayer;

class SoundHandle {
public:
	static SoundHandle make();

	bool done();
	void stop();
	void move(Vec2 newPos);

private:
	struct Data;
	std::shared_ptr<Data> data_;
	friend SoundPlayer;
};

class SoundPlayer {
public:
	struct Context;
	struct Playback;

	SoundPlayer();
	~SoundPlayer();

	void volume(float volume);
	float volume();
	void flush();

	void play(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center)
	{
		play(asset, volume, center, nullHandle_);
	}

	void play(
		SoundAsset *asset, float volume,
		std::optional<Vec2> center,
		SoundHandle handle);

	void setCenter(float x, float y);

private:
	SoundHandle nullHandle_;

	void *stream_;
	bool ok_ = false;
	std::unique_ptr<Context> context_;
};

}
