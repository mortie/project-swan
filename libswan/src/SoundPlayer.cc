#include "SoundPlayer.h"
#include "RingBuffer.h"
#include "swan/log.h"
#include "swan/util.h"

#include <SDL3/SDL.h>
#include <thread>

namespace Swan {

constexpr size_t MAX_PLAYBACKS = 64;
constexpr size_t MAX_NEW_PLAYBACKS = 32;
constexpr int CHANNELS = 2;


struct SoundHandle::Data {
	std::atomic<bool> done = false;
	std::atomic<bool> stop = false;
	std::atomic<bool> hasMoved = false;
	std::atomic<float> centerX = 0, centerY= 0;
};

SoundHandle SoundHandle::make()
{
	SoundHandle handle;
	handle.data_ = std::make_shared<Data>();
	return handle;
}

bool SoundHandle::done()
{
	return data_ && data_->done;
}

void SoundHandle::stop()
{
	if (data_) {
		data_->stop = true;
	}
}

void SoundHandle::move(Vec2 newPos)
{
	if (data_) {
		data_->centerX = newPos.x;
		data_->centerY = newPos.y;
		data_->hasMoved = true;
	}
}

struct SoundPlayer::Playback {
	std::shared_ptr<SoundHandle::Data> handle;
	SoundAsset *asset;
	float volume;
	size_t position;
	std::optional<Vec2> center;
};

struct SoundPlayer::Context {
	Playback playbacks[MAX_PLAYBACKS];
	size_t playbackCount = 0;
	bool ended = false;

	AtomicRingBuffer<Playback, MAX_NEW_PLAYBACKS> newPlaybacks;
	std::atomic<bool> end = false;
	std::atomic<bool> flush = false;
	std::atomic<float> volume = 0.5;
	std::atomic<float> centerX = 0;
	std::atomic<float> centerY = 0;
};

static void fillAudioBuffer(
	float *output,
	size_t samples,
	SoundPlayer::Context *ctx)
{
	if (ctx->flush.exchange(false)) {
		for (size_t i = 0; i < ctx->playbackCount; ++i) {
			ctx->playbacks[i].handle->done = true;
			ctx->playbacks[i].handle.reset();
		}
		ctx->playbackCount = 0;

		while (ctx->newPlaybacks.canRead()) {
			auto pb = ctx->newPlaybacks.read();
			pb.handle->done = true;
		}
	}

	float volume = ctx->volume;
	float centerX = ctx->centerX;
	float centerY = ctx->centerY;

	// Zero out the playback buffer
	memset(output, 0, samples * CHANNELS * sizeof(*output));

	if (ctx->ended) {
		return;
	}

	// Add all new playbacks
	while (ctx->newPlaybacks.canRead()) {
		auto playback = std::move(ctx->newPlaybacks.read());
		if (ctx->playbackCount >= MAX_PLAYBACKS) {
			break;
		}

		ctx->playbacks[ctx->playbackCount++] = std::move(playback);
	}

	// Sum up all playbacks into the output
	size_t idx = 0;
	while (idx < ctx->playbackCount) {
		auto &playback = ctx->playbacks[idx];

		if (!playback.handle) {
			warn << "Playback " << idx << " has null handle??";
			ctx->playbacks[idx] = ctx->playbacks[--ctx->playbackCount];
			continue;
		}

		if (playback.handle->stop) {
			ctx->playbacks[idx].handle->done = true;
			ctx->playbacks[idx] = ctx->playbacks[--ctx->playbackCount];
			continue;
		}

		if (playback.handle->hasMoved.exchange(false)) {
			playback.center->x = playback.handle->centerX;
			playback.center->y = playback.handle->centerY;
		}

		float attL = playback.volume;
		float attR = playback.volume;
		if (playback.center) {
			float distXL = std::abs(playback.center->x - (centerX - 0.8));
			float distXR = std::abs(playback.center->x - (centerX + 0.8));
			float distY = std::abs(playback.center->y - centerY);
			float distL = std::sqrt(distXL * distXL + distY * distY);
			float distR = std::sqrt(distXR * distXR + distY * distY);
			attL *= 4 / (distL + 2);
			attR *= 4 / (distR + 2);
			if (attL < 0.01 || attR < 0.01) {
				idx += 1;
				continue;
			}
		}

		bool done = false;
		size_t end = playback.position + samples;
		if (end >= playback.asset->length) {
			end = playback.asset->length;
			done = true;
		}

		// PortAudio output is interleaved
		float *dest = output;
		for (size_t i = playback.position; i < end; ++i) {
			*(dest++) += playback.asset->l[i] * attL;
			*(dest++) += playback.asset->r[i] * attR;
		}

		// Clear out the playback if it's done
		if (done) {
			ctx->playbacks[idx].handle->done = true;
			ctx->playbacks[idx] = ctx->playbacks[--ctx->playbackCount];
		}
		else {
			playback.position += samples;
			idx += 1;
		}
	}

	// Scale by volume
	float *dest = output;
	for (unsigned long i = 0; i < samples * 2; ++i) {
		*(dest)++ *= volume;
	}

	// End smoothly
	if (ctx->end) {
		float *dest = output;
		float delta = 1.0 / samples;
		float scale = 1;
		for (size_t i = 0; i < samples; ++i) {
			*(dest++) *= scale;
			*(dest++) *= scale;
			scale -= delta;
		}

		ctx->ended = true;
	}
}

static void callback(void *ptr, SDL_AudioStream *stream, int additionalBytes, int /*totalBytes*/)
{
	constexpr size_t SAMPLES_PER_BUFFER = 1024;
	size_t additionalSamples = additionalBytes / (sizeof(float) * CHANNELS);
	SoundPlayer::Context *ctx = (SoundPlayer::Context *)ptr;
	float buffer[SAMPLES_PER_BUFFER * CHANNELS];

	while (additionalSamples > 0) {
		size_t samples = additionalSamples;
		if (samples > SAMPLES_PER_BUFFER) {
			samples = SAMPLES_PER_BUFFER;
		}

		fillAudioBuffer(buffer, samples, ctx);
		SDL_PutAudioStreamData(stream, buffer, samples * sizeof(float) * CHANNELS);
		additionalSamples -= samples;
	}
}

SoundPlayer::SoundPlayer()
{
	nullHandle_ = SoundHandle::make();
	nullHandle_.data_->done = true;

	context_ = std::make_unique<Context>();

	SDL_AudioSpec spec;
	spec.channels = 2;
	spec.format = SDL_AUDIO_F32;
	spec.freq = 48000;
	auto stream = SDL_OpenAudioDeviceStream(
		SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
		callback, context_.get());
	if (!stream) {
		warn << "Failed to open audio stream: " << SDL_GetError();
		return;
	}

	SDL_ResumeAudioStreamDevice(stream);
	stream_ = stream;
}

SoundPlayer::~SoundPlayer()
{
	using namespace std::chrono_literals;

	if (stream_) {
		// Ending abruptly causes sound glitches.
		// By setting end=true, we will cause the callback
		// to fade out smoothly the next time it's called.
		context_->end = true;

		// Give it some time to fade out.
		std::this_thread::sleep_for(50ms);

		SDL_DestroyAudioStream((SDL_AudioStream *)stream_);
	}
}

void SoundPlayer::volume(float volume)
{
	context_->volume = volume;
}

float SoundPlayer::volume()
{
	return context_->volume;
}

void SoundPlayer::flush()
{
	context_->flush = true;
}

void SoundPlayer::play(
	SoundAsset *asset, float volume,
	std::optional<Vec2> center,
	SoundHandle handle)
{
	assert(handle.data_);

	if (!asset) {
		warn << "Attempt to play null asset";
		return;
	}

	if (!stream_) {
		handle.data_->done = true;
		return;
	}

	if (!context_->newPlaybacks.canWrite()) {
		warn << "Can't play sound: ring buffer full";
		handle.data_->done = true;
		return;
	}

	context_->newPlaybacks.write({
		.handle = handle.data_,
		.asset = asset,
		.volume = volume,
		.position = 0,
		.center = center,
	});
}

void SoundPlayer::setCenter(float x, float y)
{
	context_->centerX = x;
	context_->centerY = y;
}

}
