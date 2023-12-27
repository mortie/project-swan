#pragma once

#include <memory>
#include <vector>
#include <stdint.h>

#include <swan-common/constants.h>
#include <swan-common/Vector2.h>

#include "util.h"

namespace Cygnet {

struct RendererState;

struct RenderChunk {
	GLuint tex;
};

struct RenderChunkShadow {
	GLuint tex;
};

struct RenderSprite {
	GLuint tex;
	SwanCommon::Vec2 scale;
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
		int frame = 0;
		RenderSprite sprite;
	};

	struct DrawRect {
		SwanCommon::Vec2 pos;
		SwanCommon::Vec2 size = {1.0, 1.0};
		Color color = {0.6, 0.6, 0.6, 0.8};
	};

	Renderer();
	~Renderer();

	void drawChunk(DrawChunk chunk) { drawChunks_.push_back(chunk); }
	void drawChunkShadow(DrawChunkShadow chunkShadow) { drawChunkShadows_.push_back(chunkShadow); }
	void drawTile(DrawTile drawTile) { drawTiles_.push_back(drawTile); }
	void drawSprite(DrawSprite drawSprite) { drawSprites_.push_back(drawSprite); }
	void drawRect(DrawRect drawRect) { drawRects_.push_back(drawRect); }

	void draw(const RenderCamera &cam);

	void uploadTileAtlas(const void *data, int width, int height);
	void modifyTile(TileID id, const void *data);

	RenderChunk createChunk(
			TileID tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	RenderChunkShadow createChunkShadow(
			uint8_t data[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void modifyChunkShadow(
			RenderChunkShadow shadow,
			uint8_t data[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void destroyChunkShadow(RenderChunkShadow chunk);

	RenderSprite createSprite(void *data, int width, int height, int fh, int repeatFrom);
	void destroySprite(RenderSprite sprite);

	SwanCommon::Vec2 winScale() { return winScale_; }

private:
	SwanCommon::Vec2 winScale_ = {1, 1};

	std::unique_ptr<RendererState> state_;

	std::vector<DrawChunk> drawChunks_;
	std::vector<DrawChunkShadow> drawChunkShadows_;
	std::vector<DrawTile> drawTiles_;
	std::vector<DrawSprite> drawSprites_;
	std::vector<DrawRect> drawRects_;
};

}
