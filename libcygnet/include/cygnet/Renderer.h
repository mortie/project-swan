#pragma once

#include <memory>
#include <span>
#include <string_view>
#include <variant>
#include <vector>
#include <stdint.h>

#include <swan/constants.h>
#include <swan/Vector2.h>

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

struct Rect {
	Swan::Vec2 pos{0, 0};
	Swan::Vec2 size{1, 1};
};

enum class TileClip: uint8_t {
	NONE = 0,
	TOP_LEFT = 1 << 0,
	TOP_RIGHT = 1 << 1,
};

inline constexpr TileClip operator|(TileClip a, TileClip b)
{
	return TileClip(uint8_t(a) | uint8_t(b));
}

inline constexpr bool operator&(TileClip a, TileClip b)
{
	return uint8_t(a) & uint8_t(b);
}

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
	Swan::Vec2 size;
	int frameCount;
	int repeatFrom;

	operator bool() const { return tex != ~(GLuint)0; }
};

struct RenderMask {
	GLuint tex = ~(GLuint)0;
	Swan::Vec2 size;

	operator bool() const { return tex != ~(GLuint)0; }
};

struct RenderCamera {
	Swan::Vec2 pos = {0, 0};
	Swan::Vec2i size = {1, 1};
	float zoom = 1;

	RenderCamera withSize(Swan::Vec2i newSize)
	{
		return {
			.pos = pos,
			.size = newSize,
			.zoom = zoom,
		};
	}
};

enum class RenderLayer {
	BACKGROUND = 0,
	BEHIND = 1,
	NORMAL = 2,
	FOREGROUND = 3,

	MAX = FOREGROUND,
};

struct RenderProps {
	bool vflip = false;
};

class Renderer {
public:
	using TileID = uint16_t;
	static constexpr int LAYER_COUNT = (int)RenderLayer::MAX + 1;

	struct DrawChunk {
		Swan::Vec2 pos;
		RenderChunk chunk;
	};

	struct DrawChunkFluid {
		Swan::Vec2 pos;
		RenderChunkFluid fluids;
	};

	struct DrawChunkShadow {
		Swan::Vec2 pos;
		RenderChunkShadow shadow;
	};

	struct DrawTile {
		Mat3gf transform;
		TileID id;
		float brightness = 1.0;
	};

	struct DrawTileClip {
		Swan::Vec2 pos;
		TileClip clip;
	};

	struct DrawSprite {
		Mat3gf transform;
		RenderSprite sprite;
		int frame = 0;
		float opacity = 1.0;
	};

	struct DrawMask {
		Swan::Vec2 pos;
		RenderMask mask;
	};

	struct DrawGrid {
		Mat3gf transform{};
		RenderSprite sprite;
		int w;
		int h;
	};

	struct DrawParticle {
		Swan::Vec2 pos;
		Swan::Vec2 size = {1.0, 1.0};
		Color color = {1.0, 0.0, 0.0, 1.0};
	};

	struct ParticleMeta {
		Swan::Vec2 vel;
		float lifetime;
		float weight;
	};

	struct SpawnParticle {
		Swan::Vec2 pos;
		Swan::Vec2 vel;
		Swan::Vec2 size = {1.0/8, 1.0/8};
		Color color;
		float lifetime = 1;
		float weight = 1;
	};

	struct DrawRect {
		Swan::Vec2 pos;
		Swan::Vec2 size = {1.0, 1.0};
		Color outline = {0.6, 0.6, 0.6, 0.8};
		Color fill = {0.0, 0.0, 0.0, 0.0};
	};

	struct DrawText {
		TextCache &textCache;
		Swan::Vec2 pos;
		std::string_view text;
		float scale = 1;
		Color color = {1.0, 1.0, 1.0, 1.0};
	};

	struct TextSegment {
		DrawText drawText;
		TextAtlas &atlas;
		Swan::Vec2 size;
		size_t start;
		size_t end;
	};

	Renderer();
	~Renderer();

	void drawChunk(DrawChunk chunk)
	{
		drawChunks_.push_back(chunk);
	}

	void drawChunkFluid(DrawChunkFluid chunkFluid)
	{
		drawChunkFluids_.push_back(chunkFluid);
	}

	void drawChunkShadow(DrawChunkShadow chunkShadow)
	{
		drawChunkShadows_.push_back(chunkShadow);
	}

	void drawTileClip(DrawTileClip dtc)
	{
		drawTileClips_.push_back(dtc);
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

	void drawTileSprite(DrawSprite drawSprite)
	{
		drawTileSprites_.push_back(drawSprite);
	}

	void drawParticle(RenderLayer layer, DrawParticle drawParticle)
	{
		drawParticles_[(int)layer].push_back(drawParticle);
	}
	void drawParticle(DrawParticle dp)
	{
		drawParticle(RenderLayer::NORMAL, dp);
	}

	void drawTileParticle(DrawParticle drawParticle)
	{
		drawTileParticles_.push_back(drawParticle);
	}

	void drawFluidMask(DrawMask drawMask)
	{
		drawFluidMasks_.push_back(drawMask);
	}

	void spawnParticle(RenderLayer layer, SpawnParticle particle)
	{
		spawnedParticles_[(int)layer].push_back({
			.pos = particle.pos,
			.size = particle.size,
			.color = particle.color,
		});
		spawnedParticleMetas_[(int)layer].push_back({
			.vel = particle.vel,
			.lifetime = particle.lifetime,
			.weight = particle.weight,
		});
	}
	void spawnParticle(SpawnParticle particle)
	{
		spawnParticle(RenderLayer::NORMAL, particle);
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
		Swan::Vec2 size;
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

	template<typename Func>
	Rect uiView(Rect rect, Func func, Anchor anchor = Anchor::CENTER)
	{
		rect = pushUIView(rect, anchor);
		func();
		popUIView();
		return rect;
	}

	Rect pushUIView(Rect rect, Anchor anchor = Anchor::CENTER);
	void popUIView();
	bool assertUIViewStackEmpty();

	Rect currentUiView()
	{
		if (uiViewStack_.empty()) {
			return {};
		}

		return uiViewStack_.back();
	}

	void setBackgroundColor(Color color) { backgroundColor_ = color; }

	void drawUIGrid(DrawGrid drawGrid, Anchor anchor = Anchor::CENTER)
	{
		applyAnchor(
			anchor, drawGrid.transform,
			drawGrid.sprite.size.scale(drawGrid.w + 2, drawGrid.h + 2));
		drawUIElements_.push_back(drawGrid);
	}

	void drawUISprite(DrawSprite drawSprite, Anchor anchor = Anchor::CENTER)
	{
		applyAnchor(anchor, drawSprite.transform, drawSprite.sprite.size);
		drawUIElements_.push_back(drawSprite);
	}

	void drawUITile(DrawTile drawTile, Anchor anchor = Anchor::CENTER)
	{
		applyAnchor(anchor, drawTile.transform, {1, 1});
		drawUIElements_.push_back(drawTile);
	}

	void drawUIRect(DrawRect drawRect, Anchor anchor = Anchor::CENTER)
	{
		applyAnchor(anchor, drawRect.pos, drawRect.size);
		drawUIElements_.push_back(drawRect);
	}

	TextSegment prepareUIText(
		DrawText drawText,
		Anchor anchor = Anchor::CENTER);

	void drawUIText(TextSegment segment)
	{
		drawUIElements_.push_back(segment);
	}

	TextSegment &drawUIText(
		DrawText drawText,
		Anchor anchor = Anchor::CENTER)
	{
		auto segment = prepareUIText(drawText, anchor);
		drawUIText(segment);
		return std::get<TextSegment>(drawUIElements_.back());
	}

	void update(float dt);

	void clear();
	void render(const RenderCamera &cam, RenderProps props = {});
	void renderUI(const RenderCamera &cam, RenderProps props = {});

	void uploadFluidAtlas(const void *data);

	void uploadTileAtlas(const void *data, int width, int height);
	void uploadTileMap(std::span<uint16_t> tiles);
	void modifyTile(TileID id, uint16_t newAssetId);

	static constexpr size_t CHUNK_SIZE = Swan::CHUNK_WIDTH * Swan::CHUNK_HEIGHT;
	RenderChunk createChunk(
		TileID tiles[CHUNK_SIZE]);
	void modifyChunk(RenderChunk chunk, Swan::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	static constexpr size_t FLUID_CHUNK_SIZE =
		CHUNK_SIZE * Swan::FLUID_RESOLUTION * Swan::FLUID_RESOLUTION;
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

	RenderSprite createSprite(
		void *data, int width, int height,
		int fh, int repeatFrom, bool linearFiltering);
	void destroySprite(RenderSprite sprite);

	RenderMask createMask(void *data, int width, int height);
	void destroyMask(RenderMask sprite);

	void setGamma(float gamma)
	{
		gamma_ = gamma;
	}

	Swan::Vec2 winScale()
	{
		return winScale_;
	}

	void setCull(Rect rect)
	{
		cullRect_ = rect;
	}

	bool isCulled(Swan::Vec2 pos)
	{
		if (pos.x < cullRect_.pos.x - cullRect_.size.x) {
			return true;
		}
		if (pos.x > cullRect_.pos.x + cullRect_.size.x) {
			return true;
		}
		if (pos.y < cullRect_.pos.y - cullRect_.size.y) {
			return true;
		}
		if (pos.y > cullRect_.pos.y + cullRect_.size.y) {
			return true;
		}

		return false;
	}


private:
	void renderLayer(RenderLayer layer, Mat3gf camMat);
	void renderUILayer(Mat3gf camMat);
	void applyAnchor(Anchor anchor, Mat3gf &mat, Swan::Vec2 size);
	void applyAnchor(Anchor anchor, Swan::Vec2 &pos, Swan::Vec2 size);

	Rect cullRect_;
	std::unique_ptr<RendererState> state_;
	Swan::Vec2 winScale_ = {1, 1};
	float gamma_;
	Color backgroundColor_;

	std::vector<Rect> uiViewStack_ = {{{0, 0}, {1, 1}}};

	std::vector<DrawChunk> drawChunks_;
	std::vector<DrawChunkFluid> drawChunkFluids_;
	std::vector<DrawChunkShadow> drawChunkShadows_;
	std::vector<DrawTileClip> drawTileClips_;
	std::vector<DrawSprite> drawTileSprites_;
	std::vector<DrawParticle> drawTileParticles_;
	std::vector<DrawMask> drawFluidMasks_;

	std::vector<DrawTile> drawTiles_[LAYER_COUNT];
	std::vector<DrawSprite> drawSprites_[LAYER_COUNT];
	std::vector<DrawParticle> drawParticles_[LAYER_COUNT];
	std::vector<DrawRect> drawRects_[LAYER_COUNT];
	std::vector<TextSegment> drawTexts_[LAYER_COUNT];
	std::vector<TextCache::RenderedCodepoint> textBuffer_;

	using UIElement = std::variant<
		DrawGrid,
		DrawSprite,
		DrawTile,
		DrawRect,
		TextSegment>;

	std::vector<UIElement> drawUIElements_;
	std::vector<TextCache::RenderedCodepoint> textUIBuffer_;

	std::vector<DrawParticle> spawnedParticles_[LAYER_COUNT];
	std::vector<ParticleMeta> spawnedParticleMetas_[LAYER_COUNT];
};

}
