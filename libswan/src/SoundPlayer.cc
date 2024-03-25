#include "SoundPlayer.h"

#include <portaudio.h>

#include "log.h"
#include "RingBuffer.h"

namespace Swan {

constexpr size_t MAX_PLAYBACKS = 64;
constexpr size_t MAX_NEW_PLAYBACKS = 32;

struct Playback {
	std::shared_ptr<SoundPlayer::Handle> handle;
	SoundAsset *asset;
	size_t position;
};

struct SoundPlayer::Context {
	Playback playbacks[MAX_PLAYBACKS];
	size_t playbackCount = 0;
	AtomicRingBuffer<Playback, MAX_NEW_PLAYBACKS> newPlaybacks;
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

	// Zero out the playback buffer
	float *output = (float *)outputBuffer;
	memset(outputBuffer, 0, samples * CHANNELS * sizeof(*output));

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

		bool done = false;
		size_t end = playback.position + samples;
		if (end >= playback.asset->length) {
			end = playback.asset->length;
			done = true;
		}

		// PortAudio output is interleaved
		float *dest = output;
		for (size_t i = playback.position; i < end; ++i) {
			*(dest++) += playback.asset->l[i];
			*(dest++) += playback.asset->r[i];
		}

		// Clear out the playback if it's done
		if (done) {
			ctx->playbacks[idx].handle->done = true;
			ctx->playbacks[idx] = ctx->playbacks[--ctx->playbackCount];
		} else {
			playback.position += samples;
			idx += 1;
		}
	}

	return 0;
}

SoundPlayer::SoundPlayer()
{
	nullHandle_ = std::make_shared<Handle>();
	nullHandle_->done = true;

	PaError err;

	context_ = std::make_unique<Context>();

	err = Pa_Initialize();
	if (err) {
		warn << "Failed to initialize portaudio: " << Pa_GetErrorText(err);
		return;
	}

	err = Pa_OpenDefaultStream(
		&stream_, 0, 2, paFloat32, 44100, paFramesPerBufferUnspecified,
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
	if (ok_) {
		Pa_StopStream(stream_);
		Pa_CloseStream(stream_);

		// Every successful call to Pa_Initialize should be matched
		// by a call to Pa_Terminate, so this works even if there are
		// multiple SoundPlayer instances
		Pa_Terminate();
	}
}

void SoundPlayer::play(SoundAsset *asset, std::shared_ptr<Handle> handle)
{
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
		.position = 0,
	});
}

}
