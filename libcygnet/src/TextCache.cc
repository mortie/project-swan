#include "TextCache.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>
#include <stb/stb_truetype.h>
#include <swan-common/cache.h>

#include "gl.h"
#include "util.h"

namespace Cygnet {

struct FontFace {
	std::unique_ptr<uint8_t[]> content;
	stbtt_fontinfo info;
};

std::shared_ptr<FontFace> loadFontFace(const char *path)
{
	std::ifstream f{path, std::ios::binary};

	if (!f) {
		std::cerr << "Cygnet: Failed to open font file " << path << '\n';
		throw std::runtime_error("Can't open font file");
	}

	f.seekg(0, std::ios_base::end);
	size_t size = f.tellg();
	f.seekg(0, std::ios_base::beg);

	auto face = std::make_shared<FontFace>();
	face->content = std::make_unique<uint8_t[]>(size);
	f.read((char *)face->content.get(), size);

	if (stbtt_InitFont(&face->info, face->content.get(), 0) == 0) {
		std::cerr << "Cygnet: Failed to init font " << path << '\n';
		throw std::runtime_error("Can't init font");
	}

	return face;
}

TextCache::TextCache(std::shared_ptr<FontFace> face, int size)
{
	face_ = std::move(face);

	scale_ = stbtt_ScaleForPixelHeight(&face_->info, size);

	int x0, y0, x1, y1;
	stbtt_GetFontBoundingBox(&face_->info, &x0, &y0, &x1, &y1);
	atlas_.charWidth = (scale_ * (x1 - x0)) / 2 + 1;
	atlas_.charHeight = (scale_ * (y1 - y0)) / 2 + 1;
	scratchPixelBuffer_ = std::make_unique<unsigned char[]>(
		atlas_.charWidth * atlas_.charHeight);

	GLint maxSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	atlas_.sideLength = std::min(
		maxSize / std::max(atlas_.charWidth, atlas_.charHeight),
		size_t(32));
	atlas_.size = atlas_.sideLength * atlas_.sideLength;
	glGenTextures(1, &atlas_.tex);
	glCheck();

	std::cerr
		<< "Cygnet: Create text atlas, "
		<< atlas_.sideLength << "x" << atlas_.sideLength << " chars, "
		<< (atlas_.sideLength * atlas_.charWidth) << "x"
		<< (atlas_.sideLength * atlas_.charHeight) << " pix\n";

	GLint oldUnpackAlignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, atlas_.tex);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED,
		atlas_.sideLength * atlas_.charWidth,
		atlas_.sideLength * atlas_.charHeight,
		0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	cache_.reset((CacheIndex)atlas_.size);
}

TextCache::~TextCache()
{
	if (atlas_.tex) {
		glDeleteTextures(1, &atlas_.tex);
	}
}

const TextCache::RenderedCodepoint &TextCache::render(Codepoint codepoint)
{
	auto it = cachedCodepoints_.find(codepoint);

	if (it != cachedCodepoints_.end()) {
		CacheIndex idx = it->second;
		cache_.bump(idx);
		return cache_[idx];
	}

	RenderedCodepoint &rendered = [&]() -> RenderedCodepoint & {
		CacheIndex idx = cache_.nextFree();
		if (idx == cache_.null()) {
			idx = cache_.nextUsed();
			RenderedCodepoint &r = cache_[idx];
			cachedCodepoints_.erase(r.codepoint);
			cachedCodepoints_[codepoint] = idx;
			return r;
		}
		else {
			RenderedCodepoint &r = cache_[idx];
			r.textureX = (idx % atlas_.sideLength) * atlas_.charWidth;
			r.textureY = (idx / atlas_.sideLength) * atlas_.charHeight;
			cachedCodepoints_[codepoint] = idx;
			return r;
		}
	}();

	rendered.codepoint = codepoint;

	int x0, y0, x1, y1;
	stbtt_GetCodepointBitmapBox(
		&face_->info, codepoint, scale_, scale_, &x0, &y0, &x1, &y1);
	rendered.width = (x1 - x0);
	rendered.x = 0;
	rendered.y = 750 * scale_ + y0;

	if (codepoint == ' ') {
		rendered.width = 270 * scale_;
		rendered.y = 0;
	} else {
		rendered.width += 68 * scale_;
	}

	std::cerr
		<< "Cygnet: TextCache: Render code point "
		<< codepoint << "...\n";

	memset(scratchPixelBuffer_.get(), 0, atlas_.charWidth * atlas_.charHeight);
	stbtt_MakeCodepointBitmap(
		&face_->info, scratchPixelBuffer_.get(),
		atlas_.charWidth, atlas_.charHeight, atlas_.charWidth,
		scale_, scale_, codepoint);

	GLint oldUnpackAlignment;
	glGetIntegerv(GL_UNPACK_ALIGNMENT, &oldUnpackAlignment);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glBindTexture(GL_TEXTURE_2D, atlas_.tex);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, rendered.textureX, rendered.textureY, atlas_.charWidth,
		atlas_.charHeight, GL_RED, GL_UNSIGNED_BYTE, scratchPixelBuffer_.get());
	glCheck();

	glPixelStorei(GL_UNPACK_ALIGNMENT, oldUnpackAlignment);

	return rendered;
}

void TextCache::renderString(
	std::string_view s,
	std::vector<RenderedCodepoint> &out,
	SwanCommon::Vec2 &size)
{
	Codepoint prev = 0;
	size.set(0, 770 * scale_);

	auto it = s.begin();
	while (it != s.end()) {
		uint8_t ch = (uint8_t)(*(it++));
		Codepoint point;
		size_t bytes;
		if (ch <= 0x7f) {
			bytes = 0;
			point = ch;
		} else if (ch <= 0xdfu) {
			bytes = 1;
			point = ch & 0x17u;
		} else if (ch <= 0xefu) {
			bytes = 2;
			point = ch & 0x0fu;
		} else if (ch <= 0xf7u) {
			bytes = 3;
			point = ch & 0x07u;
		} else {
			bytes = 0;
			point = 0xfffdu;
		}

		for (size_t i = 0; i < bytes; ++i) {
			if (it == s.end()) {
				point = 0xfffdu;
				break;
			}

			point <<= 6;
			point |= (uint8_t)(*(it++)) & 0x3fu;
		}

		out.push_back(render(point));
		auto &rendered = out.back();
		kern(prev, rendered);
		prev = rendered.codepoint;

		size.x += rendered.width;
	}

	size.x -= 45 * scale_;
	if (size.x < 0) {
		size.x = 0;
	}
}

static bool isRound(TextCache::Codepoint ch) {
	return ch == 'e' || ch == 'o';
}

static bool isRoundLeft(TextCache::Codepoint ch) {
	return isRound(ch) ||
		ch == 'a' || ch == 'c' || ch == 'g' || ch == 'q' || ch == 'd';
}

static bool isRoundRight(TextCache::Codepoint ch) {
	return isRound(ch) || ch == 'b' || ch == 'h';
}

static bool isSmall(TextCache::Codepoint ch)
{
	return (
		ch == 'a' || ch == 'c' || ch == 'e' ||
		ch == 'g' || (ch >= 'm' && ch <= 's') ||
		(ch >= 'u' && ch <= 'z'));
}

static bool isSmallLeft(TextCache::Codepoint ch)
{
	return isSmall(ch) || ch == 'd';
}

static bool isSmallRight(TextCache::Codepoint ch)
{
	return isSmall(ch) || ch == 'b' || ch == 'h';
}

void TextCache::kern(Codepoint prev, RenderedCodepoint &rendered)
{
	Codepoint curr = rendered.codepoint;

	if (prev == 'W' && isSmallLeft(curr)) {
		rendered.x = -80 * scale_;
	}
	else if (curr == 'W' && isSmallRight(prev)) {
		rendered.x = -80 * scale_;
	}
	else if (prev == 'T' && isSmallLeft(curr)) {
		rendered.x = -180 * scale_;
	}
	else if (curr == 'T' && isSmallRight(prev)) {
		rendered.x = -180 * scale_;
	}
	else if (
			(prev == 'r' || prev == 'w' || prev == 'k') &&
			(isRoundLeft(curr))) {
		rendered.x = -60 * scale_;
	}
	else if (isRoundRight(prev) && curr == 'w') {
		rendered.x = -60 * scale_;
	}
}

}
