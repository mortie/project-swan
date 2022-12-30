#pragma once

#include <iostream>

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
};

struct RenderCamera {
	SwanCommon::Vec2 pos = {0, 0};
	SwanCommon::Vec2i size = {1, 1};
	float zoom = 1;
};

class Renderer {
public:
	using TileID = uint16_t;

	Renderer();
	~Renderer();

	void drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos);
	void drawChunkShadow(RenderChunkShadow shadow, SwanCommon::Vec2 pos);
	void drawTile(TileID id, Mat3gf mat, float brightness = 1);
	void drawSprite(RenderSprite sprite, Mat3gf mat, int y = 0);
	void drawRect(SwanCommon::Vec2 pos, SwanCommon::Vec2 size, Color color = {0.6, 0.6, 0.6, 0.8});

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

	RenderSprite createSprite(void *data, int width, int height, int fh);
	RenderSprite createSprite(void *data, int width, int height);
	void destroySprite(RenderSprite sprite);

	SwanCommon::Vec2 winScale() { return winScale_; }

private:
	struct DrawChunk {
		SwanCommon::Vec2 pos;
		RenderChunk chunk;
	};

	struct DrawShadow {
		SwanCommon::Vec2 pos;
		RenderChunkShadow shadow;
	};

	struct DrawTile {
		Mat3gf transform;
		TileID id;
		float brightness;
	};

	struct DrawSprite {
		Mat3gf transform;
		int frame;
		RenderSprite sprite;
	};

	struct DrawRect {
		SwanCommon::Vec2 pos;
		SwanCommon::Vec2 size;
		Color color;
	};

	SwanCommon::Vec2 winScale_ = {1, 1};

	std::unique_ptr<RendererState> state_;

	std::vector<DrawChunk> drawChunks_;
	std::vector<DrawShadow> drawChunkShadows_;
	std::vector<DrawTile> drawTiles_;
	std::vector<DrawSprite> drawSprites_;
	std::vector<DrawRect> drawRects_;
};

inline void Renderer::drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos) {
	drawChunks_.push_back({pos, chunk});
}

inline void Renderer::drawChunkShadow(RenderChunkShadow shadow, SwanCommon::Vec2 pos) {
	drawChunkShadows_.push_back({pos, shadow});
}

inline void Renderer::drawTile(TileID id, Mat3gf mat, float brightness) {
	drawTiles_.push_back({mat, id, brightness});
}

inline void Renderer::drawSprite(RenderSprite sprite, Mat3gf mat, int frame) {
	drawSprites_.push_back({mat, frame, sprite});
}

inline void Renderer::drawRect(SwanCommon::Vec2 pos, SwanCommon::Vec2 size, Color color) {
	drawRects_.push_back({pos, size, color});
}

}
