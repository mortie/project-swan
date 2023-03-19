#pragma once

#include <memory>
#include <unordered_map>
#include <stdint.h>

namespace Cygnet {

struct FontFace;
std::shared_ptr<FontFace> loadFontFace(const char *path);

class TextCache {
public:
	using Codepoint = uint32_t;
	using CacheIndex = uint16_t;

	struct RenderedCodepoint {
		Codepoint codepoint;
		uint16_t textureX;
		uint16_t textureY;
		uint16_t width;
		uint16_t y;
	};

	TextCache(std::shared_ptr<FontFace> face, int size);
	~TextCache();

	RenderedCodepoint &render(Codepoint codepoint);

private:
	class Impl;

	std::unique_ptr<Impl> impl_;
};

}
