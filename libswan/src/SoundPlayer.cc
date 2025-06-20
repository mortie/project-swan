#include "SoundPlayer.h"

#include <portaudio.h>
#include <thread>
#include <utility>

#include "log.h"
#include "RingBuffer.h"

namespace Swan {

constexpr size_t MAX_PLAYBACKS = 64;
constexpr size_t MAX_NEW_PLAYBACKS = 32;

struct Playback {
	std::shared_ptr<SoundPlayer::Handle> handle;
	SoundAsset *asset;
	float volume;
	size_t position;
	std::optional<std::pair<float, float>> center;
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

static int callback(
	const void * /*inputBuffer*/, void *outputBuffer,
	unsigned long samples,
	const PaStreamCallbackTimeInfo * /*timeInfo*/,
	PaStreamCallbackFlags /*statusFlags*/,
	void *userData)
{
	constexpr int CHANNELS = 2;

	SoundPlayer::Context *ctx = (SoundPlayer::Context *)userData;

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

	float *output = (float *)outputBuffer;
	float volume = ctx->volume;
	float centerX = ctx->centerX;
	float centerY = ctx->centerY;

	// Zero out the playback buffer
	memset(outputBuffer, 0, samples * CHANNELS * sizeof(*output));

	if (ctx->ended) {
		return paComplete;
	}

	// Add all new playbacks
	while (ctx->newPlaybacks.canRead()) {
		if (ctx->playbackCount >= MAX_PLAYBACKS) {
			break;
		}

		ctx->playbacks[ctx->playbackCount++] = std::move(ctx->newPlaybacks.read());
	}

	// Sum up all playbacks into the output
	size_t idx = 0;
	while (idx < ctx->playbackCount) {
		auto &playback = ctx->playbacks[idx];

		float attL = playback.volume;
		float attR = playback.volume;
		if (playback.center) {
			float distXL = std::abs(playback.center->first - (centerX - 0.8));
			float distXR = std::abs(playback.center->first - (centerX + 0.8));
			float distY = std::abs(playback.center->second - centerY);
			float distL = std::sqrt(distXL * distXL + distY * distY);
			float distR = std::sqrt(distXR * distXR + distY * distY);
			attL *= 4 / (distL + 2);
			attR *= 4 / (distR + 2);
			if (attL < 0.01 || attR < 0.01) {
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

	return paContinue;
}

SoundPlayer::SoundPlayer()
{
	nullHandle_ = std::make_shared<Handle>();
	nullHandle_->done = true;

	context_ = std::make_unique<Context>();

	PaError err;

	err = Pa_Initialize();
	if (err) {
		warn << "Failed to initialize portaudio: " << Pa_GetErrorText(err);
		return;
	}

	err = Pa_OpenDefaultStream(
		&stream_, 0, 2, paFloat32, 48000, paFramesPerBufferUnspecified,
		callback, context_.get());
	if (err) {
		warn << "Failed to open stream: " << Pa_GetErrorText(err);
		Pa_Terminate();
		return;
	}

	err = Pa_StartStream(stream_);
	if (err) {
		warn << "Failed to start stream: " << Pa_GetErrorText(err);
		Pa_CloseStream(stream_);
		Pa_Terminate();
		return;
	}

	ok_ = true;
}

SoundPlayer::~SoundPlayer()
{
	using namespace std::chrono_literals;
	if (ok_) {
		// Ending abruptly causes sound glitches.
		// By setting end=true, we will cause the callback
		// to fade out smoothly the next time it's called.
		context_->end = true;

		// Give it some time to fade out.
		std::this_thread::sleep_for(50ms);

		Pa_StopStream(stream_);
		Pa_CloseStream(stream_);

		// Every successful call to Pa_Initialize should be matched
		// by a call to Pa_Terminate, so this works even if there are
		// multiple SoundPlayer instances
		Pa_Terminate();
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
	std::optional<std::pair<float, float>> center,
	std::shared_ptr<Handle> handle)
{
	if (!asset) {
		warn << "Attempt to play null asset";
		return;
	}

	if (!ok_) {
		handle->done = true;
		return;
	}

	if (!context_->newPlaybacks.canWrite()) {
		warn << "Can't play sound: ring buffer full";
		handle->done = true;
		return;
	}

	context_->newPlaybacks.write({
		.handle = handle,
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
