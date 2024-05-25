#pragma once

#include <memory>
#include <string_view>
#include <vector>
#include <stdint.h>

#include <swan-common/constants.h>
#include <swan-common/Vector2.h>

#include "util.h"
#include "TextCache.h"

namespace Cygnet {

struct RendererState;

enum class Anchor {
	CENTER,
	TOP,
	LEFT,
	RIGHT,
	BOTTOM,
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT,
};

struct RenderChunk {
	GLuint tex = ~(GLuint)0;
};

struct RenderChunkShadow {
	GLuint tex = ~(GLuint)0;
};

struct RenderSprite {
	GLuint tex = ~(GLuint)0;
	SwanCommon::Vec2 size;
	int frameCount;
	int repeatFrom;
};

struct RenderCamera {
	SwanCommon::Vec2 pos = {0, 0};
	SwanCommon::Vec2i size = {1, 1};
	float zoom = 1;
};

class Renderer {
public:
	using TileID = uint16_t;

	struct DrawChunk {
		SwanCommon::Vec2 pos;
		RenderChunk chunk;
	};

	struct DrawChunkShadow {
		SwanCommon::Vec2 pos;
		RenderChunkShadow shadow;
	};

	struct DrawTile {
		Mat3gf transform;
		TileID id;
		float brightness = 1.0;
	};

	struct DrawSprite {
		Mat3gf transform;
		RenderSprite sprite;
		int frame = 0;
	};

	struct DrawRect {
		SwanCommon::Vec2 pos;
		SwanCommon::Vec2 size = {1.0, 1.0};
		Color outline = {0.6, 0.6, 0.6, 0.8};
		Color fill = {0.0, 0.0, 0.0, 0.0};
	};

	struct DrawText {
		TextCache &textCache;
		Mat3gf transform;
		std::string_view text;
		Color color = {1.0, 1.0, 1.0, 1.0};
	};

	struct TextSegment {
		DrawText drawText;
		TextAtlas &atlas;
		SwanCommon::Vec2 size;
		size_t start;
		size_t end;
	};

	Renderer();
	~Renderer();

	void drawChunk(DrawChunk chunk)
	{
		drawChunks_.push_back(chunk);
	}

	void drawChunkShadow(DrawChunkShadow chunkShadow)
	{
		drawChunkShadows_.push_back(chunkShadow);
	}

	void drawTile(DrawTile drawTile)
	{
		drawTiles_.push_back(drawTile);
	}

	void drawSprite(DrawSprite drawSprite)
	{
		drawSprites_.push_back(drawSprite);
	}

	void drawRect(DrawRect drawRect)
	{
		drawRects_.push_back(drawRect);
	}

	TextSegment &drawText(DrawText drawText)
	{
		SwanCommon::Vec2 size;
		size_t start = textBuffer_.size();

		drawText.textCache.renderString(drawText.text, textBuffer_, size);
		drawTexts_.push_back({
			.drawText = drawText,
			.atlas = drawText.textCache.atlas_,
			.size = size / 128,
			.start = start,
			.end = textBuffer_.size(),
		});
		return drawTexts_.back();
	}

	void drawUISprite(DrawSprite drawSprite, Anchor anchor = Anchor::CENTER)
	{
		drawUISprites_.push_back(drawSprite);
		drawUISpritesAnchors_.push_back(anchor);
	}

	void drawUITile(DrawTile drawTile, Anchor anchor = Anchor::CENTER)
	{
		drawUITiles_.push_back(drawTile);
		drawUITilesAnchors_.push_back(anchor);
	}

	TextSegment &drawUIText(DrawText drawText, Anchor anchor = Anchor::CENTER)
	{
		SwanCommon::Vec2 size;
		size_t start = textUIBuffer_.size();

		drawText.textCache.renderString(drawText.text, textUIBuffer_, size);
		drawUITexts_.push_back({
			.drawText = drawText,
			.atlas = drawText.textCache.atlas_,
			.size = size / 64,
			.start = start,
			.end = textUIBuffer_.size(),
		});
		drawUITextsAnchors_.push_back(anchor);
		return drawUITexts_.back();
	}

	void render(const RenderCamera &cam);

	void renderUI(const RenderCamera &cam);

	void uploadTileAtlas(const void *data, int width, int height);
	void modifyTile(TileID id, const void *data);

	RenderChunk createChunk(
		TileID tiles[SwanCommon::CHUNK_WIDTH *SwanCommon::CHUNK_HEIGHT]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	RenderChunkShadow createChunkShadow(
		uint8_t data[SwanCommon::CHUNK_WIDTH *SwanCommon::CHUNK_HEIGHT]);
	void modifyChunkShadow(
		RenderChunkShadow shadow,
		uint8_t data[SwanCommon::CHUNK_WIDTH *SwanCommon::CHUNK_HEIGHT]);
	void destroyChunkShadow(RenderChunkShadow chunk);

	RenderSprite createSprite(void *data, int width, int height, int fh, int repeatFrom);
	void destroySprite(RenderSprite sprite);

	SwanCommon::Vec2 winScale()
	{
		return winScale_;
	}

private:
	SwanCommon::Vec2 winScale_ = {1, 1};

	std::unique_ptr<RendererState> state_;

	std::vector<DrawChunk> drawChunks_;
	std::vector<DrawChunkShadow> drawChunkShadows_;
	std::vector<DrawTile> drawTiles_;
	std::vector<DrawSprite> drawSprites_;
	std::vector<DrawRect> drawRects_;
	std::vector<TextSegment> drawTexts_;
	std::vector<TextCache::RenderedCodepoint> textBuffer_;

	std::vector<DrawSprite> drawUISprites_;
	std::vector<Anchor> drawUISpritesAnchors_;

	std::vector<DrawTile> drawUITiles_;
	std::vector<Anchor> drawUITilesAnchors_;
	std::vector<TextSegment> drawUITexts_;
	std::vector<Anchor> drawUITextsAnchors_;
	std::vector<TextCache::RenderedCodepoint> textUIBuffer_;
};

}
