#include "Renderer.h"

#include <iostream>
#include <stdio.h>
#include <SDL_opengles2.h>
#include <swan-common/constants.h>

#include "shaders.h"
#include "GlWrappers.h"
#include "TileAtlas.h"
#include "util.h"

namespace Cygnet {

struct TexturedProg: public GlProgram {
	using GlProgram::GlProgram;

	GLint transform = uniformLoc("camera");
	GLint position = attribLoc("position");
	GLint texCoord = attribLoc("texCoord");
	GLint tex = uniformLoc("tex");
};

struct RendererState {
	GlVxShader texturedVx{Shaders::texturedVx};
	GlFrShader texturedFr{Shaders::texturedFr};

	TexturedProg texturedProg{texturedVx, texturedFr};

	TileAtlas atlas;
	GlTexture atlasTex;

	Mat3gf camera;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {}

Renderer::~Renderer() = default;

void Renderer::draw() {
	state_->texturedProg.use();

	float tw =  (6 * SwanCommon::TILE_SIZE) / (float)state_->atlasTex.width();
	GLfloat vertexes[] = {
		-0.5f,  0.5f, // pos 0: top left
		 0.0f,  0.0f, // tex 0: top left
		-0.5f, -0.5f, // pos 1: bottom left
		 0.0f,  1.0f, // tex 1: bottom left
		 0.5f, -0.5f, // pos 2: bottom right
		 tw,    1.0f, // tex 2: bottom right
		 0.5f,  0.5f, // pos 3: top right
		 tw,    0.0f, // tex 3: top right
	};

	GLushort indexes[] = {
		0, 1, 2, // top left -> bottom left -> bottom right
		2, 3, 0, // bottom right -> top right -> top left
	};

	state_->camera.translate(0.01, 0);
	glUniformMatrix3fv(state_->texturedProg.transform, 1, GL_TRUE, state_->camera.data());
	glVertexAttribPointer(state_->texturedProg.position, 2, GL_FLOAT, GL_FALSE,
			4 * sizeof(GLfloat), vertexes);
	glVertexAttribPointer(state_->texturedProg.texCoord, 2, GL_FLOAT, GL_FALSE,
			4 * sizeof(GLfloat), &vertexes[2]);
	glCheck();

	glEnableVertexAttribArray(state_->texturedProg.position);
	glEnableVertexAttribArray(state_->texturedProg.texCoord);
	glCheck();

	state_->atlasTex.bind();
	glUniform1i(state_->texturedProg.tex, 0);
	glCheck();

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indexes);
	glCheck();
}

void Renderer::registerTileTexture(size_t tileId, const void *data, size_t len) {
	state_->atlas.addTile(tileId, data, len);
}

void Renderer::uploadTileTexture() {
	size_t w, h;
	const unsigned char *data = state_->atlas.getImage(&w, &h);
	state_->atlasTex.upload(w, h, (void *)data, GL_RGBA, GL_UNSIGNED_BYTE);
	std::cerr << "Uploaded image of size " << w << 'x' << h << '\n';

	FILE *f = fopen("lol.rgb", "w");
	fwrite(data, 1, w * h * 4, f);
	fclose(f);
}

}
