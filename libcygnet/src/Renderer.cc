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

struct SpriteProg: public GlProgram {
	template<typename... T>
	SpriteProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~SpriteProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint transform = uniformLoc("transform");
	GLint vertex = attribLoc("vertex");
	GLint texCoord = attribLoc("texCoord");
	GLint tex = uniformLoc("tex");

	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		-0.5f,  0.5f, // pos 0: top left
		 0.0f,  0.0f, // tex 0: top left
		-0.5f, -0.5f, // pos 1: bottom left
		 0.0f,  1.0f, // tex 1: bottom left
		 0.5f, -0.5f, // pos 2: bottom right
		 1.0f,  1.0f, // tex 2: bottom right
		 0.5f, -0.5f, // pos 2: bottom right
		 1.0f,  1.0f, // tex 2: bottom right
		 0.5f,  0.5f, // pos 3: top right
		 1.0f,  0.0f, // tex 3: top right
		-0.5f,  0.5f, // pos 0: top left
		 0.0f,  0.0f, // tex 0: top left
	};

	void enable() {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(vertex);
		glEnableVertexAttribArray(texCoord);
		glCheck();
	}

	void disable() {
		glDisableVertexAttribArray(vertex);
		glDisableVertexAttribArray(texCoord);
		glCheck();
	}

	void init() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	void deinit()  {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}
};

struct ChunkProg: public GlProgram {
	template<typename... T>
	ChunkProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~ChunkProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint pos = uniformLoc("pos");
	GLint vertex = attribLoc("vertex");
	GLint tileTex = uniformLoc("tileTex");
	GLint tileTexSize = uniformLoc("tileTexSize");
	GLint tiles = uniformLoc("tiles");

	GLuint vbo;

	static constexpr float ch = (float)SwanCommon::CHUNK_HEIGHT;
	static constexpr float cw = (float)SwanCommon::CHUNK_WIDTH;
	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f, -ch ,  // pos 1: bottom left
		cw,   -ch,   // pos 2: bottom right
		cw,   -ch,   // pos 2: bottom right
		cw,    0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	void enable() {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(vertex);
		glCheck();
	}

	void disable() {
		glDisableVertexAttribArray(vertex);
		glCheck();
	}

	void init() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	void deinit() {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}
};

struct RendererState {
	GlVxShader spriteVx{Shaders::spriteVx};
	GlFrShader spriteFr{Shaders::spriteFr};
	GlVxShader chunkVx{Shaders::chunkVx};
	GlFrShader chunkFr{Shaders::chunkFr};

	SpriteProg spriteProg{spriteVx, spriteFr};
	ChunkProg chunkProg{chunkVx, chunkFr};

	TileAtlas atlas;
	GlTexture atlasTex;

	Mat3gf camera;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {}

Renderer::~Renderer() = default;

void Renderer::draw() {
	state_->chunkProg.enable();

	state_->camera.reset().translate(-0.5, 0.5).scale(0.25, 0.25);
	glUniformMatrix3fv(state_->chunkProg.camera, 1, GL_TRUE, state_->camera.data());
	glCheck();

	GLint tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT] = {0};
	tiles[0] = 3;
	tiles[1] = 1;
	glUniform1iv(state_->chunkProg.tiles, sizeof(tiles) / sizeof(*tiles), tiles);
	glCheck();

	glUniform2f(state_->chunkProg.tileTexSize,
			(float)(int)(state_->atlasTex.width() / SwanCommon::TILE_SIZE),
			(float)(int)(state_->atlasTex.height() / SwanCommon::TILE_SIZE));
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,state_->atlasTex.id()); // Necessary?
	glUniform1i(state_->chunkProg.tileTex, 0);
	glCheck();

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glCheck();

	state_->chunkProg.disable();

	/*
	state_->spriteProg.enable();

	state_->camera.translate(-0.00001, 0);
	glUniformMatrix3fv(state_->spriteProg.transform, 1, GL_TRUE, Mat3gf::IDENTITY.data());
	glUniformMatrix3fv(state_->spriteProg.camera, 1, GL_TRUE, state_->camera.data());
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,state_->atlasTex.id()); // Necessary?
	glUniform1i(state_->spriteProg.tex, 0);
	glCheck();

	glDrawArrays(GL_TRIANGLES, 0, 6);
	glCheck();

	state_->spriteProg.disable();
	*/
}

void Renderer::registerTileTexture(size_t tileId, const void *data, size_t len) {
	state_->atlas.addTile(tileId, data, len);
}

void Renderer::uploadTileTexture() {
	size_t w, h;
	const unsigned char *data = state_->atlas.getImage(&w, &h);
	state_->atlasTex.upload(w, h, (void *)data, GL_RGBA, GL_UNSIGNED_BYTE);
	std::cerr << "Uploaded image of size " << w << 'x' << h << '\n';
}

}
