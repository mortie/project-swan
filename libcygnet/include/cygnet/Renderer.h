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

struct RenderChunkFluid {
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

enum class RenderLayer {
	BACKGROUND = 0,
	BEHIND = 1,
	NORMAL = 2,
	FOREGROUND = 3,

	MAX = FOREGROUND,
};

class Renderer {
public:
	using TileID = uint16_t;
	static constexpr int LAYER_COUNT = (int)RenderLayer::MAX + 1;

	struct DrawChunk {
		SwanCommon::Vec2 pos;
		RenderChunk chunk;
	};

	struct DrawChunkFluid {
		SwanCommon::Vec2 pos;
		RenderChunkFluid fluids;
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

	struct DrawParticle {
		SwanCommon::Vec2 pos;
		SwanCommon::Vec2 size = {1.0, 1.0};
		Color color = {1.0, 0.0, 0.0, 1.0};
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

	void drawChunk(RenderLayer layer, DrawChunk chunk)
	{
		drawChunks_[(int)layer].push_back(chunk);
	}
	void drawChunk(DrawChunk chunk)
	{
		drawChunk(RenderLayer::NORMAL, chunk);
	}

	void drawChunkFluid(RenderLayer layer, DrawChunkFluid chunkFluid)
	{
		drawChunkFluids_[(int)layer].push_back(chunkFluid);
	}
	void drawChunkFluid(DrawChunkFluid chunkFluid)
	{
		drawChunkFluid(RenderLayer::NORMAL, chunkFluid);
	}

	void drawChunkShadow(RenderLayer layer, DrawChunkShadow chunkShadow)
	{
		drawChunkShadows_[(int)layer].push_back(chunkShadow);
	}
	void drawChunkShadow(DrawChunkShadow chunkShadow)
	{
		drawChunkShadow(RenderLayer::NORMAL, chunkShadow);
	}

	void drawTile(RenderLayer layer, DrawTile drawTile)
	{
		drawTiles_[(int)layer].push_back(drawTile);
	}
	void drawTile(DrawTile dt)
	{
		drawTile(RenderLayer::NORMAL, dt);
	}

	void drawSprite(RenderLayer layer, DrawSprite drawSprite)
	{
		drawSprites_[(int)layer].push_back(drawSprite);
	}
	void drawSprite(DrawSprite ds)
	{
		drawSprite(RenderLayer::NORMAL, ds);
	}

	void drawParticle(RenderLayer layer, DrawParticle drawParticle)
	{
		drawParticles_[(int)layer].push_back(drawParticle);
	}
	void drawParticle(DrawParticle dp)
	{
		drawParticle(RenderLayer::NORMAL, dp);
	}

	void drawRect(RenderLayer layer, DrawRect drawRect)
	{
		drawRects_[(int)layer].push_back(drawRect);
	}
	void drawRect(DrawRect dr)
	{
		drawRect(RenderLayer::NORMAL, dr);
	}

	TextSegment &drawText(RenderLayer layer, DrawText drawText)
	{
		SwanCommon::Vec2 size;
		size_t start = textBuffer_.size();

		drawText.textCache.renderString(drawText.text, textBuffer_, size);
		drawTexts_[(int)layer].push_back({
			.drawText = drawText,
			.atlas = drawText.textCache.atlas_,
			.size = size / 128,
			.start = start,
			.end = textBuffer_.size(),
		});
		return drawTexts_[(int)layer].back();
	}
	TextSegment &drawText(DrawText dt)
	{
		return drawText(RenderLayer::NORMAL, dt);
	}

	void drawUISprite(RenderLayer layer, DrawSprite drawSprite, Anchor anchor = Anchor::CENTER)
	{
		drawUISprites_[(int)layer].push_back(drawSprite);
		drawUISpritesAnchors_[(int)layer].push_back(anchor);
	}
	void drawUISprite(DrawSprite drawSprite, Anchor anchor = Anchor::CENTER)
	{
		drawUISprite(RenderLayer::NORMAL, drawSprite, anchor);
	}

	void drawUITile(RenderLayer layer, DrawTile drawTile, Anchor anchor = Anchor::CENTER)
	{
		drawUITiles_[(int)layer].push_back(drawTile);
		drawUITilesAnchors_[(int)layer].push_back(anchor);
	}
	void drawUITile(DrawTile drawTile, Anchor anchor = Anchor::CENTER)
	{
		drawUITile(RenderLayer::NORMAL, drawTile, anchor);
	}

	TextSegment &drawUIText(RenderLayer layer, DrawText drawText, Anchor anchor = Anchor::CENTER)
	{
		SwanCommon::Vec2 size;
		size_t start = textUIBuffer_.size();

		drawText.textCache.renderString(drawText.text, textUIBuffer_, size);
		drawUITexts_[(int)layer].push_back({
			.drawText = drawText,
			.atlas = drawText.textCache.atlas_,
			.size = size / 64,
			.start = start,
			.end = textUIBuffer_.size(),
		});
		drawUITextsAnchors_[(int)layer].push_back(anchor);
		return drawUITexts_[(int)layer].back();
	}
	TextSegment &drawUIText(DrawText drawText, Anchor anchor = Anchor::CENTER)
	{
		return drawUIText(RenderLayer::NORMAL, drawText, anchor);
	}

	void render(const RenderCamera &cam);

	void renderUI(const RenderCamera &cam);

	void uploadFluidAtlas(const void *data);

	void uploadTileAtlas(const void *data, int width, int height);
	void modifyTile(TileID id, const void *data);

	static constexpr size_t CHUNK_SIZE = SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT;
	RenderChunk createChunk(
		TileID tiles[CHUNK_SIZE]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	static constexpr size_t FLUID_CHUNK_SIZE =
		CHUNK_SIZE * SwanCommon::FLUID_RESOLUTION * SwanCommon::FLUID_RESOLUTION;
	RenderChunkFluid createChunkFluid(
		uint8_t data[FLUID_CHUNK_SIZE]);
	void modifyChunkFluid(
		RenderChunkFluid fluid,
		uint8_t data[FLUID_CHUNK_SIZE]);
	void destroyChunkFluid(RenderChunkFluid fluid);

	RenderChunkShadow createChunkShadow(
		uint8_t data[CHUNK_SIZE]);
	void modifyChunkShadow(
		RenderChunkShadow shadow,
		uint8_t data[CHUNK_SIZE]);
	void destroyChunkShadow(RenderChunkShadow shadow);

	RenderSprite createSprite(void *data, int width, int height, int fh, int repeatFrom);
	void destroySprite(RenderSprite sprite);

	SwanCommon::Vec2 winScale()
	{
		return winScale_;
	}

private:
	void renderLayer(RenderLayer layer, Mat3gf camMat);
	void renderUILayer(RenderLayer layer, SwanCommon::Vec2 scale, Mat3gf camMat);

	SwanCommon::Vec2 winScale_ = {1, 1};

	std::unique_ptr<RendererState> state_;

	std::vector<DrawChunk> drawChunks_[LAYER_COUNT];
	std::vector<DrawChunkFluid> drawChunkFluids_[LAYER_COUNT];
	std::vector<DrawChunkShadow> drawChunkShadows_[LAYER_COUNT];
	std::vector<DrawTile> drawTiles_[LAYER_COUNT];
	std::vector<DrawSprite> drawSprites_[LAYER_COUNT];
	std::vector<DrawParticle> drawParticles_[LAYER_COUNT];
	std::vector<DrawRect> drawRects_[LAYER_COUNT];
	std::vector<TextSegment> drawTexts_[LAYER_COUNT];
	std::vector<TextCache::RenderedCodepoint> textBuffer_;

	std::vector<DrawSprite> drawUISprites_[LAYER_COUNT];
	std::vector<Anchor> drawUISpritesAnchors_[LAYER_COUNT];

	std::vector<DrawTile> drawUITiles_[LAYER_COUNT];
	std::vector<Anchor> drawUITilesAnchors_[LAYER_COUNT];
	std::vector<TextSegment> drawUITexts_[LAYER_COUNT];
	std::vector<Anchor> drawUITextsAnchors_[LAYER_COUNT];
	std::vector<TextCache::RenderedCodepoint> textUIBuffer_;
};

}
