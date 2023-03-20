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

#include "Blend.glsl.h"
#include "Chunk.glsl.h"
#include "ChunkShadow.glsl.h"
#include "Rect.glsl.h"
#include "Sprite.glsl.h"
#include "Tile.glsl.h"

namespace Cygnet {

struct BlendProg: public GlProg<Shader::Blend> {
	Shader::Blend locs{id()};
	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		-1.0f, -1.0f, 0.0f, 0.0f, // pos 0: top left
		-1.0f,  1.0f, 0.0f, 1.0f, // pos 1: bottom left
		 1.0f,  1.0f, 1.0f, 1.0f, // pos 2: bottom right
		 1.0f,  1.0f, 1.0f, 1.0f, // pos 2: bottom right
		 1.0f, -1.0f, 1.0f, 0.0f, // pos 3: top right
		-1.0f, -1.0f, 0.0f, 0.0f, // pos 0: top left
	};

	BlendProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~BlendProg()  {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(GLuint tex) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(
			locs.attrVertex, 2, GL_FLOAT, GL_FALSE,
			4 * sizeof(GLfloat), (void *)0);
		glVertexAttribPointer(
			locs.attrTexCoord, 2, GL_FLOAT, GL_FALSE,
			4 * sizeof(GLfloat), (void *)(2 * sizeof(GLfloat)));
		glEnableVertexAttribArray(locs.attrVertex);
		glEnableVertexAttribArray(locs.attrTexCoord);
		glCheck();

		glUniform1i(locs.uniTex, 0);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glCheck();

		glDisableVertexAttribArray(locs.attrVertex);
		glDisableVertexAttribArray(locs.attrTexCoord);
		glCheck();
	}
};

struct ChunkProg: public GlProg<Shader::Chunk> {
	GLuint vbo;

	static constexpr float ch = (float)SwanCommon::CHUNK_HEIGHT;
	static constexpr float cw = (float)SwanCommon::CHUNK_WIDTH;
	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  ch,   // pos 1: bottom left
		cw,    ch,   // pos 2: bottom right
		cw,    ch,   // pos 2: bottom right
		cw,    0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	ChunkProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~ChunkProg() {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(
			const std::vector<Renderer::DrawChunk> &drawChunks, const Mat3gf &cam,
			GLuint atlasTex, SwanCommon::Vec2 atlasTexSize) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(shader.attrVertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(shader.attrVertex);
		glCheck();

		glUniform1i(shader.uniTileAtlas, 0);
		glUniform1i(shader.uniTiles, 1);

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform2f(
			shader.uniTileAtlasSize,
			atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTex);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		for (auto [pos, chunk]: drawChunks) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, chunk.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		glDisableVertexAttribArray(shader.attrVertex);
		glCheck();
	}
};

struct ChunkShadowProg: public GlProg<Shader::ChunkShadow> {
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

	ChunkShadowProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~ChunkShadowProg() {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(const std::vector<Renderer::DrawChunkShadow> &drawChunkShadows, const Mat3gf &cam) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(shader.attrVertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(shader.attrVertex);
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto [pos, shadow]: drawChunkShadows) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, shadow.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		glDisableVertexAttribArray(shader.attrVertex);
		glCheck();
	}
};

struct RectProg: public GlProg<Shader::Rect> {
	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  1.0f, // pos 1: bottom left
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	RectProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~RectProg()  {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(const std::vector<Renderer::DrawRect> &drawRects, const Mat3gf &cam) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(shader.attrVertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(shader.attrVertex);
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		for (auto [pos, size, color]: drawRects) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glUniform2f(shader.uniSize, size.x, size.y);
			glUniform4f(shader.uniColor, color.r, color.g, color.b, color.a);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		glDisableVertexAttribArray(shader.attrVertex);
		glCheck();
	}
};

struct SpriteProg: public GlProg<Shader::Sprite> {
	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  1.0f, // pos 1: bottom left
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	SpriteProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~SpriteProg()  {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(const std::vector<Renderer::DrawSprite> &drawSprites, const Mat3gf &cam) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(shader.attrVertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(shader.attrVertex);
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto [mat, frame, sprite]: drawSprites) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, mat.data());
			glUniform2f(shader.uniFrameSize, sprite.scale.x, sprite.scale.y);
			glUniform2f(shader.uniFrameInfo, sprite.frameCount, frame);
			glBindTexture(GL_TEXTURE_2D, sprite.tex);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		glDisableVertexAttribArray(shader.attrVertex);
		glCheck();
	}
};

struct TileProg: public GlProg<Shader::Tile> {
	GLuint vbo;

	static constexpr GLfloat vertexes[] = {
		0.0f,  0.0f, // pos 0: top left
		0.0f,  1.0f, // pos 1: bottom left
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  1.0f, // pos 2: bottom right
		1.0f,  0.0f, // pos 3: top right
		0.0f,  0.0f, // pos 0: top left
	};

	TileProg() {
		glGenBuffers(1, &vbo);
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
		glCheck();
	}

	~TileProg() {
		glDeleteBuffers(1, &vbo);
		glCheck();
	}

	void draw(
			const std::vector<Renderer::DrawTile> &drawTiles, const Mat3gf &cam,
			GLuint atlasTex, SwanCommon::Vec2 atlasTexSize) {
		glUseProgram(id());
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(shader.attrVertex, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
		glEnableVertexAttribArray(shader.attrVertex);
		glCheck();

		glUniform1i(shader.uniTileAtlas, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform2f(shader.uniTileAtlasSize, atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTex);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		for (auto [mat, id, brightness]: drawTiles) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, mat.data());
			glUniform1f(shader.uniTileID, id);
			glUniform1f(shader.uniBrightness, brightness);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glCheck();
		}

		glDisableVertexAttribArray(shader.attrVertex);
		glCheck();
	}
};

struct RendererState {
	BlendProg blendProg{};
	ChunkProg chunkProg{};
	ChunkShadowProg chunkShadowProg{};
	RectProg rectProg{};
	SpriteProg spriteProg{};
	TileProg tileProg{};

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

	state_->chunkProg.draw(drawChunks_, camMat, state_->atlasTex, state_->atlasTexSize);
	drawChunks_.clear();

	state_->tileProg.draw(drawTiles_, camMat, state_->atlasTex, state_->atlasTexSize);
	drawTiles_.clear();

	state_->spriteProg.draw(drawSprites_, camMat);
	drawSprites_.clear();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	state_->blendProg.draw(state_->offscreenTex);

	state_->chunkShadowProg.draw(drawChunkShadows_, camMat);
	drawChunkShadows_.clear();

	state_->rectProg.draw(drawRects_, camMat);
	drawRects_.clear();
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
				GL_TEXTURE_2D, 0, GL_RG,
				SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
				0, GL_RG, GL_UNSIGNED_BYTE, tiles);
		glCheck();
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
				GL_TEXTURE_2D, 0, GL_RG,
				SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
				0, GL_RG, GL_UNSIGNED_BYTE, buf);
		glCheck();
	}

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
				GL_RG, GL_UNSIGNED_BYTE, &id);
	} else if constexpr (std::endian::native == std::endian::big) {
		uint8_t buf[] = { (uint8_t)(id & 0xff), (uint8_t)((id & 0xff00) >> 8) };
		glTexSubImage2D(
				GL_TEXTURE_2D, 0, pos.x, pos.y, 1, 1,
				GL_RG, GL_UNSIGNED_BYTE, buf);
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
			GL_TEXTURE_2D, 0, GL_RED,
			SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
			0, GL_RED, GL_UNSIGNED_BYTE, data);
	glCheck();

	return shadow;
}

void Renderer::modifyChunkShadow(
		RenderChunkShadow shadow,
		uint8_t data[SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT]) {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow.tex);

	glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RED,
			SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
			0, GL_RED, GL_UNSIGNED_BYTE, data);
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
