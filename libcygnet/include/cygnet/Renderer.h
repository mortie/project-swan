#pragma once

#include <memory>
#include <vector>
#include <stdint.h>

#include <swan-common/constants.h>
#include <swan-common/Vector2.h>

#include "util.h"

namespace Cygnet {

struct RendererState;

struct RenderSprite {
	GLuint tex;
};

struct RenderChunk {
	GLuint tex;
};

struct RenderCamera {
	SwanCommon::Vec2 pos;
	SwanCommon::Vec2i size;
	float zoom;
};

class Renderer {
public:
	using TileID = uint16_t;

	Renderer();
	~Renderer();

	void drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos);
	void drawSprite(RenderSprite sprite, Mat3gf mat);
	void drawSprite(RenderSprite sprite, SwanCommon::Vec2 pos);
	void drawSpriteFlipped(RenderSprite chunk, SwanCommon::Vec2 pos);

	void draw(const RenderCamera &cam);

	void registerTileTexture(TileID tileId, const void *data, size_t len);
	void uploadTileTexture();

	RenderChunk createChunk(
			TileID tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

	RenderSprite createSprite(uint8_t *rgb, int width, int height);
	void destroySprite(RenderSprite sprite);

private:
	std::unique_ptr<RendererState> state_;

	std::vector<std::pair<SwanCommon::Vec2, RenderChunk>> draw_chunks_;
	std::vector<std::pair<Mat3gf, RenderSprite>> draw_sprites_;
};

inline void Renderer::drawChunk(RenderChunk chunk, SwanCommon::Vec2 pos) {
	draw_chunks_.emplace_back(pos, chunk);
}

inline void Renderer::drawSprite(RenderSprite sprite, Mat3gf mat) {
	draw_sprites_.emplace_back(mat, sprite);
}

inline void Renderer::drawSprite(RenderSprite sprite, SwanCommon::Vec2 pos) {
	draw_sprites_.emplace_back(Mat3gf{}.translate(pos), sprite);
}

inline void Renderer::drawSpriteFlipped(RenderSprite sprite, SwanCommon::Vec2 pos) {
	draw_sprites_.emplace_back(Mat3gf{}.translate(pos).scale({ -1, 1 }), sprite);
}

}
