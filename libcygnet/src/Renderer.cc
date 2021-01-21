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

struct ChunkShadowProg: public GlProgram {
	template<typename... T>
	ChunkShadowProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~ChunkShadowProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint pos = uniformLoc("pos");
	GLint vertex = attribLoc("vertex");
	GLint tex = uniformLoc("tex");

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

	void deinit() {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}
};

struct TileProg: public GlProgram {
	template<typename... T>
	TileProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~TileProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint transform = uniformLoc("transform");
	GLint vertex = attribLoc("vertex");
	GLint tileAtlas = uniformLoc("tileAtlas");
	GLint tileAtlasSize = uniformLoc("tileAtlasSize");
	GLint tileID = uniformLoc("tileID");

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

		glUniform1i(tileAtlas, 0);
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

struct RectProg: public GlProgram {
	template<typename... T>
	RectProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~RectProg() { deinit(); }

	GLint camera = uniformLoc("camera");
	GLint pos = uniformLoc("pos");
	GLint size = uniformLoc("size");
	GLint vertex = attribLoc("vertex");

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

struct BlendProg: public GlProgram {
	template<typename... T>
	BlendProg(const T &... shaders): GlProgram(shaders...) { init(); }
	~BlendProg() { deinit(); }

	GLint vertex = attribLoc("vertex");
	GLint texCoord = attribLoc("texCoord");
	GLint tex = uniformLoc("tex");

	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, // pos 0: top left
		-1.0f,  1.0f, 0.0f, 1.0f, // pos 1: bottom left
		 1.0f,  1.0f, 1.0f, 1.0f, // pos 2: bottom right
		 1.0f,  1.0f, 1.0f, 1.0f, // pos 2: bottom right
		 1.0f, -1.0f, 1.0f, 0.0f, // pos 3: top right
		-1.0f, -1.0f, 0.0f, 0.0f, // pos 0: top left
	};

	void enable() {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(vertex, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)0);
		glVertexAttribPointer(texCoord, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(vertex);
		glEnableVertexAttribArray(texCoord);
		glCheck();

		glUniform1i(tex, 0);
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

	void deinit()  {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}
};

struct RendererState {
	GlVxShader chunkVx{Shaders::chunkVx};
	GlFrShader chunkFr{Shaders::chunkFr};
	GlVxShader chunkShadowVx{Shaders::chunkShadowVx};
	GlFrShader chunkShadowFr{Shaders::chunkShadowFr};
	GlVxShader tileVx{Shaders::tileVx};
	GlFrShader tileFr{Shaders::tileFr};
	GlVxShader spriteVx{Shaders::spriteVx};
	GlFrShader spriteFr{Shaders::spriteFr};
	GlVxShader rectVx{Shaders::rectVx};
	GlFrShader rectFr{Shaders::rectFr};
	GlVxShader blendVx{Shaders::blendVx};
	GlFrShader blendFr{Shaders::blendFr};

	ChunkProg chunkProg{chunkVx, chunkFr};
	ChunkShadowProg chunkShadowProg{chunkShadowVx, chunkShadowFr};
	TileProg tileProg{tileVx, tileFr};
	SpriteProg spriteProg{spriteVx, spriteFr};
	RectProg rectProg{rectVx, rectFr};
	BlendProg blendProg{blendVx, blendFr};

	SwanCommon::Vec2i screenSize;
	GLuint offscreenFramebuffer = 0;
	GLuint offscreenTex = 0;
	GLuint atlasTex = 0;
	SwanCommon::Vec2 atlasTexSize;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>()) {
	glGenTextures(1, &state_->atlasTex);
	glCheck();

	glGenTextures(1, &state_->offscreenTex);
	glCheck();
}

Renderer::~Renderer() {
	glDeleteFramebuffers(1, &state_->offscreenFramebuffer);
	glDeleteTextures(1, &state_->offscreenFramebuffer);
	glDeleteTextures(1, &state_->atlasTex);
}

void Renderer::draw(const RenderCamera &cam) {
	Mat3gf camMat;

	camMat.translate(-cam.pos);

	if (cam.size.y > cam.size.x) {
		float ratio = (float)cam.size.y / (float)cam.size.x;
		winScale_ = {1/ratio, 1};
		camMat.scale({cam.zoom * ratio, -cam.zoom});
	} else {
		float ratio = (float)cam.size.x / (float)cam.size.y;
		winScale_ = {1, 1/ratio};
		camMat.scale({cam.zoom, -cam.zoom * ratio});
	}

	auto &chunkProg = state_->chunkProg;
	auto &chunkShadowProg = state_->chunkShadowProg;
	auto &tileProg = state_->tileProg;
	auto &spriteProg = state_->spriteProg;
	auto &rectProg = state_->rectProg;
	auto &blendProg = state_->blendProg;

	if (state_->screenSize != cam.size) {
		state_->screenSize = cam.size;
		glBindTexture(GL_TEXTURE_2D, state_->offscreenTex);
		glCheck();
		glTexImage2D(
				GL_TEXTURE_2D, 0, GL_RGBA, cam.size.x, cam.size.y, 0,
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheck();

		glGenFramebuffers(1, &state_->offscreenFramebuffer);
		glCheck();
		glBindFramebuffer(GL_FRAMEBUFFER, state_->offscreenFramebuffer);
		glCheck();
		glFramebufferTexture2D(
				GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
				state_->offscreenTex, 0);
		glCheck();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, state_->offscreenFramebuffer);
	glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			state_->offscreenTex, 0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

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
		tileProg.enable();
		glUniformMatrix3fv(tileProg.camera, 1, GL_TRUE, camMat.data());
		glCheck();

		glUniform2f(tileProg.tileAtlasSize, state_->atlasTexSize.x, state_->atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		for (auto [mat, id]: drawTiles_) {
			glUniformMatrix3fv(tileProg.transform, 1, GL_TRUE, mat.data());
			glUniform1f(tileProg.tileID, id);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		drawTiles_.clear();
		tileProg.disable();
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

	{
		rectProg.enable();
		glUniformMatrix3fv(rectProg.camera, 1, GL_TRUE, camMat.data());
		glCheck();

		for (auto [pos, size]: drawRects_) {
			glUniform2f(rectProg.pos, pos.x, pos.y);
			glUniform2f(rectProg.size, size.x, size.y);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		drawRects_.clear();
		rectProg.disable();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	{
		blendProg.enable();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, state_->offscreenTex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glCheck();
		blendProg.disable();
	}

	{
		chunkShadowProg.enable();
		glUniformMatrix3fv(chunkShadowProg.camera, 1, GL_TRUE, camMat.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto [pos, shadow]: drawChunkShadows_) {
			glUniform2f(chunkShadowProg.pos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, shadow.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		drawChunkShadows_.clear();
		chunkShadowProg.disable();
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
	RenderChunk chunk;
	glGenTextures(1, &chunk.tex);
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, chunk.tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

RenderChunkShadow Renderer::createChunkShadow(
		uint8_t data[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]) {
	RenderChunkShadow shadow;
	glGenTextures(1, &shadow.tex);
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow.tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheck();

	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_LUMINANCE,
			SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
			0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glCheck();

	return shadow;
}

void Renderer::modifyChunkShadow(
		RenderChunkShadow shadow,
		uint8_t data[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow.tex);

	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_LUMINANCE,
			SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
			0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
	glCheck();
}

void Renderer::destroyChunkShadow(RenderChunkShadow shadow) {
	glDeleteTextures(1, &shadow.tex);
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
