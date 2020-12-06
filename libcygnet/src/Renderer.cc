#include "Renderer.h"

#include <SDL_opengles2.h>

#include "shaders.h"
#include "GlWrappers.h"
#include "TileAtlas.h"
#include "util.h"

namespace Cygnet {

struct TexturedProg: public GlProgram {
	using GlProgram::GlProgram;

	GLint position = attribLoc("position");
	GLint texCoord = attribLoc("texCoord");
	GLint tex = uniformLoc("tex");
};

struct SolidColorProg: public GlProgram {
	using GlProgram::GlProgram;

	GLint position = attribLoc("position");
	GLint color = uniformLoc("color");
};

struct RendererState {
	GlVxShader basicVx{Shaders::basicVx};
	GlVxShader texturedVx{Shaders::texturedVx};
	GlFrShader solidColorFr{Shaders::solidColorFr};
	GlFrShader texturedFr{Shaders::texturedFr};

	TexturedProg texturedProg{texturedVx, texturedFr};
	SolidColorProg solidColorProg{basicVx, solidColorFr};

	TileAtlas atlas;
	GlTexture atlasTex;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {}

Renderer::~Renderer() = default;

void Renderer::draw() {
	state_->texturedProg.use();
}

void Renderer::registerTileTexture(size_t tileId, const void *data, size_t len) {
	state_->atlas.addTile(tileId, data, len);
}

void Renderer::uploadTileTexture() {
	size_t w, h;
	const unsigned char *data = state_->atlas.getImage(&w, &h);
	state_->atlasTex.upload(w, h, (void *)data, GL_RGBA, GL_UNSIGNED_BYTE);
}

}
