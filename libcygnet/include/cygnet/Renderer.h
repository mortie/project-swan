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

struct RenderSprite {
	GLuint tex;
	SwanCommon::Vec2 scale;
	int frameCount;
};

struct RenderTile {
	uint16_t id;
};

struct RenderCamera {
	SwanCommon::Vec2 pos;
	SwanCommon::Vec2i size;
	float zoom = 1;
};

class Renderer {
public:
	using TileID = uint16_t;

	Renderer();
	~Renderer();

	void drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos);
	void drawSprite(RenderSprite sprite, Mat3gf mat, int y = 0);

	void draw(const RenderCamera &cam);

	void uploadTileAtlas(const void *data, int width, int height);
	void modifyTile(TileID id, const void *data);

	RenderChunk createChunk(
			TileID tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	RenderSprite createSprite(void *data, int width, int height, int fh);
	RenderSprite createSprite(void *data, int width, int height);
	void destroySprite(RenderSprite sprite);

private:
	struct DrawChunk {
		SwanCommon::Vec2 pos;
		RenderChunk chunk;
	};

	struct DrawSprite {
		Mat3gf transform;
		int frame;
		RenderSprite sprite;
	};

	std::unique_ptr<RendererState> state_;

	std::vector<DrawChunk> drawChunks_;
	std::vector<DrawSprite> drawSprites_;
};

inline void Renderer::drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos) {
	drawChunks_.push_back({pos, chunk});
}

inline void Renderer::drawSprite(RenderSprite sprite, Mat3gf mat, int frame) {
	drawSprites_.push_back({mat, frame, sprite});
}

}
