#pragma once

#include <memory>
#include <stdint.h>
#include <string_view>
#include <unordered_map>
#include <swan/cache.h>
#include <vector>

#include "util.h"

namespace Cygnet {

struct FontFace;
std::shared_ptr<FontFace> loadFontFace(const char *path);

struct TextAtlas {
	size_t sideLength;
	size_t charWidth;
	size_t charHeight;
	size_t size;
	GLuint tex;
};

class TextCache {
public:
	using Codepoint = uint32_t;
	using CacheIndex = uint16_t;

	struct RenderedCodepoint {
		Codepoint codepoint;
		uint16_t textureX;
		uint16_t textureY;
		uint16_t width;
		int16_t x;
		int16_t y;
	};

	TextCache(std::shared_ptr<FontFace> face, int size);
	~TextCache();

	const RenderedCodepoint &render(Codepoint codepoint);
	void renderString(
		std::string_view s,
		std::vector<RenderedCodepoint> &out,
		Swan::Vec2 &size);

	void kern(Codepoint prev, RenderedCodepoint &rendered);

	TextAtlas atlas_;

private:
	Swan::LruCache<RenderedCodepoint, CacheIndex> cache_;
	std::unordered_map<Codepoint, CacheIndex> cachedCodepoints_;
	std::unique_ptr<unsigned char[]> scratchPixelBuffer_;
	std::shared_ptr<FontFace> face_;
	float scale_;
};

}
