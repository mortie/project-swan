#include "Renderer.h"

#include <iostream>
#include <stdio.h>
#include <swan-common/constants.h>
#include <string.h>

// std::endian was originally in type_traits, was moved to bit
#include <type_traits>
#include <bit>

#include "gl.h"
#include "shaders.h"
#include "GlWrappers.h"
#include "TileAtlas.h"
#include "util.h"

namespace Cygnet {

struct ChunkProg: public GlProgram {
	template<typename... T>
	ChunkProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~ChunkProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint pos = uniformLoc("pos");
	GLint vertex = attribLoc("vertex");
	GLint tileAtlas = uniformLoc("tileAtlas");
	GLint tileAtlasSize = uniformLoc("tileAtlasSize");
	GLint tiles = uniformLoc("tiles");

	GLuint vbo;

	static constexpr float ch = (float)SwanCommon::CHUNK_HEIGHT;
	static constexpr float cw = (float)SwanCommon::CHUNK_WIDTH;
	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  ch ,  // pos 1: bottom left
		cw,    ch,   // pos 2: bottom right
		cw,    ch,   // pos 2: bottom right
		cw,    0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	void enable() {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(vertex);
		glCheck();

		glUniform1i(tileAtlas, 0);
		glUniform1i(tiles, 1);
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

struct SpriteProg: public GlProgram {
	template<typename... T>
	SpriteProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~SpriteProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint transform = uniformLoc("transform");
	GLint frameSize = uniformLoc("frameSize");
	GLint frameInfo = uniformLoc("frameInfo");
	GLint vertex = attribLoc("vertex");
	GLint tex = uniformLoc("tex");

	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  1.0f, // pos 1: bottom left
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	void enable() {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(vertex);
		glCheck();

		glUniform1i(tex, 0);
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

	void deinit()  {
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

	GLuint atlasTex;
	SwanCommon::Vec2 atlasTexSize;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {
	glGenTextures(1, &state_->atlasTex);
	glCheck();
}

Renderer::~Renderer() = default;

void Renderer::draw(const RenderCamera &cam) {
	Mat3gf camMat;

	// Make the matrix translate to { -camX, -camY }, fix up the aspect ratio,
	// flip the Y axis so that positive Y direction is down, and scale according to zoom.
	float ratio = (float)cam.size.y / (float)cam.size.x;
	camMat.translate(-cam.pos);
	camMat.scale({cam.zoom * ratio, -cam.zoom});

	auto &chunkProg = state_->chunkProg;
	auto &spriteProg = state_->spriteProg;

	{
		chunkProg.enable();
		glUniformMatrix3fv(chunkProg.camera, 1, GL_TRUE, camMat.data());
		glCheck();

		glUniform2f(chunkProg.tileAtlasSize, state_->atlasTexSize.x, state_->atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		for (auto [pos, chunk]: drawChunks_) {
			glUniform2f(chunkProg.pos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, chunk.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		drawChunks_.clear();
		chunkProg.disable();
	}

	{
		spriteProg.enable();
		glUniformMatrix3fv(spriteProg.camera, 1, GL_TRUE, camMat.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto [mat, frame, sprite]: drawSprites_) {
			glUniformMatrix3fv(spriteProg.transform, 1, GL_TRUE, mat.data());
			glUniform2f(spriteProg.frameSize, sprite.scale.x, sprite.scale.y);
			glUniform2f(spriteProg.frameInfo, sprite.frameCount, frame);
			glBindTexture(GL_TEXTURE_2D, sprite.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		drawSprites_.clear();
		spriteProg.disable();
	}
}

void Renderer::uploadTileAtlas(const void *data, int width, int height) {
	glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();

	state_->atlasTexSize = {
		(float)(int)(width / SwanCommon::TILE_SIZE),
		(float)(int)(height / SwanCommon::TILE_SIZE) };
}

void Renderer::modifyTile(TileID id, const void *data) {
	int w = (int)state_->atlasTexSize.x;
	int x = id % w;
	int y = id / w;
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
	glTexSubImage2D(
			GL_TEXTURE_2D, 0, x * SwanCommon::TILE_SIZE, y * SwanCommon::TILE_SIZE,
			SwanCommon::TILE_SIZE, SwanCommon::TILE_SIZE,
			GL_RGBA, GL_UNSIGNED_BYTE, data);
	glCheck();
}

RenderChunk Renderer::createChunk(
		TileID tiles[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]) {
	// TODO: Maybe don't do this here? Maybe instead store the buffer and
	// upload the texture in the draw method?
	// The current approach needs createChunk to be called on the graphics thread.

	RenderChunk chunk;
	glGenTextures(1, &chunk.tex);
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, chunk.tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();

	static_assert(
			std::endian::native == std::endian::big ||
			std::endian::native == std::endian::little,
			"Expected either big or little endian");

	if constexpr (std::endian::native == std::endian::little) {
		glTexImage2D(
				GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
				SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
				0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, tiles);
	} else if constexpr (std::endian::native == std::endian::big) {
		uint8_t buf[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT * 2];
		for (size_t y = 0; y < SwanCommon::CHUNK_HEIGHT; ++y) {
			for (size_t x = 0; x < SwanCommon::CHUNK_WIDTH; ++x) {
				size_t dst = y * SwanCommon::CHUNK_WIDTH * 2 + x * 2;
				size_t src = y * SwanCommon::CHUNK_WIDTH + x;
				buf[dst + 0] = tiles[src] & 0xff;
				buf[dst + 1] = (tiles[src] & 0xff00) >> 8;
			}
		}

		glTexImage2D(
				GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA,
				SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
				0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buf);
	}

	glCheck();
	return chunk;
}

void Renderer::modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, chunk.tex);
	glCheck();

	static_assert(
			std::endian::native == std::endian::big ||
			std::endian::native == std::endian::little,
			"Expected either big or little endian");

	if constexpr (std::endian::native == std::endian::little) {
		glTexSubImage2D(
				GL_TEXTURE_2D, 0, pos.x, pos.y, 1, 1,
				GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, &id);
	} else if constexpr (std::endian::native == std::endian::big) {
		uint8_t buf[] = { (uint8_t)(id & 0xff), (uint8_t)((id & 0xff00) >> 8) };
		glTexSubImage2D(
				GL_TEXTURE_2D, 0, pos.x, pos.y, 1, 1,
				GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, buf);
	}

	glCheck();
}

void Renderer::destroyChunk(RenderChunk chunk) {
	glDeleteTextures(1, &chunk.tex);
	glCheck();
}

RenderSprite Renderer::createSprite(void *data, int width, int height, int fh) {
	RenderSprite sprite;
	sprite.scale = {
		(float)width / SwanCommon::TILE_SIZE,
		(float)fh / SwanCommon::TILE_SIZE };
	sprite.frameCount = height / fh;
	glGenTextures(1, &sprite.tex);
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sprite.tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheck();

	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, width, height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glCheck();

	return sprite;
}

RenderSprite Renderer::createSprite(void *data, int width, int height) {
	return createSprite(data, width, height, height);
}

void Renderer::destroySprite(RenderSprite sprite) {
	glDeleteTextures(1, &sprite.tex);
}

}
