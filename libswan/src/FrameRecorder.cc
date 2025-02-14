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
Swan::Vec2i FrameRecorder::size() { return {}; }
void FrameRecorder::flush() {}
bool FrameRecorder::openEncoder(int, int, int, const char *) { return false; }

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
#include <libavformat/avformat.h>
}


#include "log.h"
#include "util.h"

namespace Swan {

struct FrameRecorder::Impl {
	struct EncCtx {
		AVFormatContext *fmtCtx = nullptr;
		AVStream *vstream = nullptr;
		AVCodecContext *codecCtx = nullptr;
	};

	bool running = false;

	EncCtx enc;

	AVFrame *rgbFrame = nullptr;
	AVFrame *yuvFrame = nullptr;
	AVPacket *pkt = nullptr;
	SwsContext *swsCtx = nullptr;
	int64_t pts = 0;

	FILE *f = nullptr;

	GLuint fbo = 0;
	GLuint fboTex = 0;
	GLint screenFBO = 0;
	int viewport[4] = {0, 0, 0, 0};
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

	// Width and height must be a power of 2
	if (w % 2) {
		w -= 1;
	}
	if (h % 2) {
		h -= 1;
	}

	impl_ = std::make_unique<Impl>();

	if (!openEncoder(w, h, fps, path)) {
		return false;
	}

	impl_->rgbFrame = av_frame_alloc();
	if (!impl_->rgbFrame) {
		warn << "Failed to allocate RGB frame";
		return false;
	}

	impl_->rgbFrame->format = AV_PIX_FMT_RGBA;
	impl_->rgbFrame->width = impl_->enc.codecCtx->width;
	impl_->rgbFrame->height = impl_->enc.codecCtx->height;
	if (av_frame_get_buffer(impl_->rgbFrame, 0) < 0) {
		warn << "Could not allocate RGB frame data";
		return false;
	}

	impl_->yuvFrame = av_frame_alloc();
	if (!impl_->yuvFrame) {
		warn << "Failed to allocate YUV frame";
		return false;
	}

	impl_->yuvFrame->duration = 1000;
	impl_->yuvFrame->format = impl_->enc.codecCtx->pix_fmt;
	impl_->yuvFrame->width = impl_->enc.codecCtx->width;
	impl_->yuvFrame->height = impl_->enc.codecCtx->height;
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

	if (impl_->fbo) {
		glDeleteFramebuffers(1, &impl_->fbo);
		impl_->fboTex = 0;
	}

	if (impl_->fboTex) {
		glDeleteTextures(1, &impl_->fboTex);
		impl_->fboTex = 0;
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

	if (impl_->enc.codecCtx) {
		avcodec_free_context(&impl_->enc.codecCtx);
	}

	if (impl_->enc.vstream) {
		// Don't do anything here, the steram should be freed
		// when freeing the format context
	}

	if (impl_->enc.fmtCtx) {
		avformat_free_context(impl_->enc.fmtCtx);
	}

	impl_.reset();
}

void FrameRecorder::beginFrame(Cygnet::Color color)
{
	glGetIntegerv(GL_VIEWPORT, impl_->viewport);

	glBindFramebuffer(GL_FRAMEBUFFER, impl_->fbo);
	glClearColor(color.r, color.g, color.b, color.a);
	glClear(GL_COLOR_BUFFER_BIT);

	auto s = size();
	glViewport(0, 0, s.x, s.y);
}

void FrameRecorder::endFrame()
{
	SWAN_DEFER(glBindFramebuffer(GL_FRAMEBUFFER, impl_->screenFBO));

	glViewport(
		impl_->viewport[0], impl_->viewport[1],
		impl_->viewport[2], impl_->viewport[3]);

	if (av_frame_make_writable(impl_->rgbFrame) < 0) {
		warn << "Failed to make RGB frame writeable!";
		return;
	}

	if (av_frame_make_writable(impl_->yuvFrame) < 0) {
		warn << "Failed to make YUV frame writeable!";
		return;
	}

	auto s = size();
	glReadPixels(
		0, 0, s.x, s.y, 
		GL_RGBA,  GL_UNSIGNED_BYTE, impl_->rgbFrame->data[0]);

	if (sws_scale_frame(impl_->swsCtx, impl_->yuvFrame, impl_->rgbFrame) < 0) {
		warn << "Failed to convert RGB to YUV!";
		return;
	}

	impl_->yuvFrame->pts = (impl_->pts++) * 1000;
	if (avcodec_send_frame(impl_->enc.codecCtx, impl_->yuvFrame) < 0) {
		warn << "Failed to send frame to encoder!";
		return;
	}

	auto *pkt = impl_->pkt;
	while (true) {
		int ret;
		if ((ret = avcodec_receive_packet(impl_->enc.codecCtx, pkt)) < 0) {
			if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
				char buf[32];
				av_make_error_string(buf, sizeof(buf), ret);
				warn << "Encoding error: " << buf;
			}
			break;
		}

		if (av_interleaved_write_frame(impl_->enc.fmtCtx, pkt) < 0) {
			warn << "Failed to write frame";
		}
	}

	// 'running' indicates that we are successfully encoding frames,
	// and will cause a flush to be triggered once encoding ends.
	impl_->running = true;
}

Swan::Vec2i FrameRecorder::size()
{
	return {
		impl_->enc.codecCtx->width,
		impl_->enc.codecCtx->height,
	};
}

bool FrameRecorder::openEncoder(int w, int h, int fps, const char *path)
{
	auto &enc = impl_->enc;

	const AVOutputFormat *fmt = av_guess_format(nullptr, path, nullptr);
	if (!fmt) {
		warn << "Failed to guess format from path";
		return false;
	}

	enc.fmtCtx = nullptr;
	avformat_alloc_output_context2(&enc.fmtCtx, nullptr, nullptr, path);
	if (!enc.fmtCtx) {
		warn << "Failed to allocate output context";
		return false;
	}

	enc.vstream = avformat_new_stream(enc.fmtCtx, nullptr);
	if (!enc.vstream) {
		warn << "Failed to allocate video stream";
		return false;
	}

	const AVCodec *encoder = avcodec_find_encoder(fmt->video_codec);
	if (!encoder) {
		warn << "Failed to find encoder for codec";
		return false;
	}

	enc.codecCtx = avcodec_alloc_context3(encoder);
	if (!enc.codecCtx) {
		warn << "Failed to alloc codec context";
		return false;
	}

	enc.codecCtx->width = w;
	enc.codecCtx->height = h;
	enc.codecCtx->time_base = AVRational(1, fps * 1000);
	enc.codecCtx->framerate = AVRational(fps, 1);
	enc.codecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	enc.codecCtx->bit_rate = 2 * 1000 * 1000;

	if (avcodec_open2(enc.codecCtx, encoder, nullptr) < 0) {
		warn << "Failed to open codec";
		return false;
	}

	if (avcodec_parameters_from_context(enc.vstream->codecpar, enc.codecCtx) < 0) {
		warn << "Failed to set codec parameters";
		return false;
	}

	enc.vstream->time_base = enc.codecCtx->time_base;
	enc.vstream->avg_frame_rate = enc.codecCtx->framerate;
	av_dump_format(enc.fmtCtx, 0, path, 1);

	if (avio_open(&enc.fmtCtx->pb, path, AVIO_FLAG_WRITE) < 0) {
		warn << "Failed to open output file '" << path << "'";
		return false;
	}

	if (avformat_write_header(enc.fmtCtx, nullptr) < 0) {
		warn << "Failed to write header";
		return false;
	}

	return true;
}

void FrameRecorder::flush()
{
	if (avcodec_send_frame(impl_->enc.codecCtx, nullptr) < 0) {
		warn << "Failed to send frame to encoder!";
		return;
	}

	auto *pkt = impl_->pkt;
	while (true) {
		int ret;
		if ((ret = avcodec_receive_packet(impl_->enc.codecCtx, pkt)) < 0) {
			if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
				char buf[32];
				av_make_error_string(buf, sizeof(buf), ret);
				warn << "Encoding error while flushing: " << buf;
			}
			break;
		}

		if (av_interleaved_write_frame(impl_->enc.fmtCtx, pkt) < 0) {
			warn << "Failed to write frame";
		}
	}

	av_write_trailer(impl_->enc.fmtCtx);
}

}

#endif
