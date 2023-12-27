#include "TextCache.h"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
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

class TextCache::Impl {
public:
	using Codepoint = TextCache::Codepoint;
	using CacheIndex = uint16_t;
	using RenderedComponent = TextCache::RenderedCodepoint;

	Impl(std::shared_ptr<FontFace> face, int fontSize): face_(std::move(face))
	{
		scale_ = stbtt_ScaleForPixelHeight(&face_->info, fontSize);

		int x0, y0, x1, y1;
		stbtt_GetFontBoundingBox(&face_->info, &x0, &y0, &x1, &y1);
		charWidth_ = scale_ * (x1 - x0) + 1;
		charHeight_ = scale_ * (y1 - y0) + 1;
		scratchPixelBuffer_ = std::make_unique<unsigned char[]>(charWidth_ * charHeight_);

		GLint maxSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
		dims_ = std::min(maxSize / std::max(charWidth_, charHeight_), size_t(32));
		size_ = dims_ * dims_;
		glGenTextures(1, &tex_);
		glCheck();

		glBindTexture(GL_TEXTURE_2D, tex_);
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED, dims_ * charWidth_, dims_ * charHeight_,
			0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

		cache_.reset((CacheIndex)size_);
	}

	~Impl()
	{
		if (tex_) {
			glDeleteTextures(1, &tex_);
		}
	}

	RenderedCodepoint &render(Codepoint codepoint)
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
				r.textureX = (idx % dims_) * charWidth_;
				r.textureY = (idx / dims_) * charHeight_;
				cachedCodepoints_[codepoint] = idx;
				return r;
			}
		}();

		rendered.codepoint = codepoint;

		int x0, y0, x1, y1;
		stbtt_GetCodepointBitmapBox(&face_->info, codepoint, scale_, scale_, &x0, &y0, &x1, &y1);
		rendered.width = x1 - x0;
		rendered.y = y0 * scale_;

		stbtt_MakeCodepointBitmap(
			&face_->info, scratchPixelBuffer_.get(), charWidth_, charHeight_, charWidth_,
			scale_, scale_, codepoint);

		glBindTexture(GL_TEXTURE_2D, tex_);
		glTexSubImage2D(
			GL_TEXTURE_2D, 0, rendered.textureX, rendered.textureY, rendered.width, charHeight_,
			GL_RED, GL_UNSIGNED_BYTE, scratchPixelBuffer_.get());
		glCheck();

		return rendered;
	}

private:
	std::shared_ptr<FontFace> face_;

	float scale_;
	size_t dims_;
	size_t size_;
	size_t charWidth_, charHeight_;
	GLuint tex_ = 0;

	SwanCommon::LruCache<RenderedCodepoint, CacheIndex> cache_;
	std::unordered_map<Codepoint, CacheIndex> cachedCodepoints_;
	std::unique_ptr<unsigned char[]> scratchPixelBuffer_;
};

TextCache::TextCache(std::shared_ptr<FontFace> face, int fontSize):
	impl_(std::make_unique<Impl>(std::move(face), fontSize))
{}

TextCache::~TextCache() = default;

TextCache::RenderedCodepoint &TextCache::render(uint32_t codepoint)
{
	return impl_->render(codepoint);
}

}
