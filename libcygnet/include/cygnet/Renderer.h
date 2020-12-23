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

	void drawChunk(SwanCommon::Vec2 pos, RenderChunk chunk);

	void draw(const RenderCamera &cam);

	void registerTileTexture(TileID tileId, const void *data, size_t len);
	void uploadTileTexture();

	RenderChunk createChunk(
			TileID tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]);
	void modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id);
	void destroyChunk(RenderChunk chunk);

private:
	std::unique_ptr<RendererState> state_;

	std::vector<std::pair<SwanCommon::Vec2, RenderChunk>> draw_chunks_;
};

inline void Renderer::drawChunk(SwanCommon::Vec2 pos, RenderChunk chunk) {
	draw_chunks_.emplace_back(pos, chunk);
}

}
