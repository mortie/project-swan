#pragma once

#include <memory>

namespace Cygnet {

struct RendererState;

class Renderer {
public:
	Renderer();
	~Renderer();

	void draw();

	void registerTileTexture(size_t tileId, const void *data, size_t len);
	void uploadTileTexture();

private:
	std::unique_ptr<RendererState> state_;
};

}
