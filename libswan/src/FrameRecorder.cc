#include "FrameRecorder.h"

#ifndef SWAN_FFMPEG_AVAILABLE

namespace Swan {

struct FrameRecorder::Impl {};

FrameRecorder::FrameRecorder() = default;
FrameRecorder::~FrameRecorder() = default;

bool FrameRecorder::isAvailable() { return false; }
bool FrameRecorder::begin(int, int, int, const char *) { return false; }
void FrameRecorder::end() {}
void FrameRecorder::beginFrame(Cygnet::Color) {}
void FrameRecorder::endFrame() {}
void FrameRecorder::flush() {}

}

#else

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <cygnet/gl.h>
#include <cygnet/util.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
#include <libswscale/swscale.h>
}


#include "log.h"
#include "util.h"

namespace Swan {

struct FrameRecorder::Impl {
	bool running = false;

	FILE *f = nullptr;
	AVCodecContext *ctx = nullptr;
	AVFrame *rgbFrame = nullptr;
	AVFrame *yuvFrame = nullptr;
	int64_t pts = 0;
	AVPacket *pkt = nullptr;

	SwsContext *swsCtx = nullptr;

	GLuint fbo = 0;
	GLuint fboTex = 0;
	GLint screenFBO = 0;
};

FrameRecorder::FrameRecorder() = default;
FrameRecorder::~FrameRecorder()
{
	end();
}

bool FrameRecorder::isAvailable() { return true; }

bool FrameRecorder::begin(int w, int h, int fps, const char *path)
{
	end();

	// Width must be a power of 2
	if (w % 2) {
		w -= 1;
	}

	impl_ = std::make_unique<Impl>();

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &impl_->screenFBO);

	glGenTextures(1, &impl_->fboTex);
	Cygnet::glCheck();
	glBindTexture(GL_TEXTURE_2D, impl_->fboTex);
	Cygnet::glCheck();
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	Cygnet::glCheck();

	glGenFramebuffers(1, &impl_->fbo);
	Cygnet::glCheck();
	glBindFramebuffer(GL_FRAMEBUFFER, impl_->fbo);
	Cygnet::glCheck();

	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		impl_->fboTex, 0);
	Cygnet::glCheck();

	impl_->f = fopen(path, "wb");
	if (!impl_->f) {
		warn << "Failed to open '" << path << "': " << strerror(errno);
		return false;
	}

	const AVCodec *codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		warn << "Codec not found";
		return false;
	}

	impl_->ctx = avcodec_alloc_context3(codec);
	if (!impl_->ctx) {
		warn << "Failed to allocate codec context";
		return false;
	}

	impl_->ctx->width = w;
	impl_->ctx->height = h;
	impl_->ctx->time_base = AVRational{1, fps};
	impl_->ctx->framerate = AVRational{fps, 1};
	impl_->ctx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (avcodec_open2(impl_->ctx, codec, nullptr) < 0) {
		warn << "Failed to open codec";
		return false;
	}

	impl_->rgbFrame = av_frame_alloc();
	if (!impl_->rgbFrame) {
		warn << "Failed to allocate RGB frame";
		return false;
	}

	impl_->rgbFrame->format = AV_PIX_FMT_RGBA;
	impl_->rgbFrame->width = impl_->ctx->width;
	impl_->rgbFrame->height = impl_->ctx->height;
	if (av_frame_get_buffer(impl_->rgbFrame, 0) < 0) {
		warn << "Could not allocate RGB frame data";
		return false;
	}

	impl_->yuvFrame = av_frame_alloc();
	if (!impl_->yuvFrame) {
		warn << "Failed to allocate YUV frame";
		return false;
	}

	impl_->yuvFrame->format = impl_->ctx->pix_fmt;
	impl_->yuvFrame->width = impl_->ctx->width;
	impl_->yuvFrame->height = impl_->ctx->height;
	if (av_frame_get_buffer(impl_->yuvFrame, 0) < 0) {
		warn << "Could not allocate yuv frame data";
		return false;
	}

	impl_->pkt = av_packet_alloc();
	if (!impl_->pkt) {
		warn << "Could not allocate packet";
		return false;
	}

	impl_->swsCtx = sws_getContext(
		w, h, AV_PIX_FMT_RGBA,
		w, h, AV_PIX_FMT_YUV420P,
		SWS_BICUBIC, nullptr, nullptr, nullptr);
	if (!impl_->swsCtx) {
		warn << "Could not allocate swscale context";
		return false;
	}

	return true;
}

void FrameRecorder::end()
{
	if (!impl_) {
		return;
	}

	if (impl_->running) {
		flush();
	}

	if (impl_->swsCtx) {
		sws_freeContext(impl_->swsCtx);
	}

	if (impl_->pkt) {
		av_packet_free(&impl_->pkt);
	}

	if (impl_->yuvFrame) {
		av_frame_free(&impl_->yuvFrame);
	}

	if (impl_->rgbFrame) {
		av_frame_free(&impl_->rgbFrame);
	}

	if (impl_->f) {
		fclose(impl_->f);
	}

	if (impl_->ctx) {
		avcodec_free_context(&impl_->ctx);
	}

	if (impl_->fbo) {
		glDeleteFramebuffers(1, &impl_->fbo);
		impl_->fboTex = 0;
	}

	if (impl_->fboTex) {
		glDeleteTextures(1, &impl_->fboTex);
		impl_->fboTex = 0;
	}

	impl_.reset();
}

void FrameRecorder::beginFrame(Cygnet::Color color)
{
	glBindFramebuffer(GL_FRAMEBUFFER, impl_->fbo);
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);
}

void FrameRecorder::endFrame()
{
	SWAN_DEFER(glBindFramebuffer(GL_FRAMEBUFFER, impl_->screenFBO));

	if (av_frame_make_writable(impl_->rgbFrame) < 0) {
		warn << "Failed to make RGB frame writeable!";
		return;
	}

	if (av_frame_make_writable(impl_->yuvFrame) < 0) {
		warn << "Failed to make YUV frame writeable!";
		return;
	}

	glReadPixels(
		0, 0, impl_->ctx->width, impl_->ctx->height, 
		GL_RGBA,  GL_UNSIGNED_BYTE, impl_->rgbFrame->data[0]);

	if (sws_scale_frame(impl_->swsCtx, impl_->yuvFrame, impl_->rgbFrame) < 0) {
		warn << "Failed to convert RGB to YUV!";
		return;
	}

	impl_->yuvFrame->pts = impl_->pts++;
	if (avcodec_send_frame(impl_->ctx, impl_->yuvFrame) < 0) {
		warn << "Failed to send frame to encoder!";
		return;
	}

	auto *pkt = impl_->pkt;
	while (true) {
		int ret;
		if ((ret = avcodec_receive_packet(impl_->ctx, pkt)) < 0) {
			if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
				char buf[32];
				av_make_error_string(buf, sizeof(buf), ret);
				warn << "Encoding error: " << buf;
			}
			break;
		}

		fwrite(impl_->pkt->data, 1, pkt->size, impl_->f);
		av_packet_unref(pkt);
	}

	// 'running' indicates that we are successfully encoding frames,
	// and will cause a flush to be triggered once encoding ends.
	impl_->running = true;
}

void FrameRecorder::flush()
{
	if (avcodec_send_frame(impl_->ctx, nullptr) < 0) {
		warn << "Failed to send frame to encoder!";
		return;
	}

	auto *pkt = impl_->pkt;
	while (true) {
		int ret;
		if ((ret = avcodec_receive_packet(impl_->ctx, pkt)) < 0) {
			if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
				char buf[32];
				av_make_error_string(buf, sizeof(buf), ret);
				warn << "Encoding error while flushing: " << buf;
			}
			break;
		}

		fwrite(impl_->pkt->data, 1, pkt->size, impl_->f);
		av_packet_unref(pkt);
	}
}

}

#endif
