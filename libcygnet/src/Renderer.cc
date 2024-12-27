#include "Renderer.h"

#include <cassert>
#include <stdio.h>
#include <swan-common/constants.h>
#include <string.h>
#include <span>

#include "GlWrappers.h"
#include "util.h"

#include "Blend.glsl.h"
#include "Chunk.glsl.h"
#include "ChunkShadow.glsl.h"
#include "ChunkFluid.glsl.h"
#include "Rect.glsl.h"
#include "Sprite.glsl.h"
#include "Text.glsl.h"
#include "Tile.glsl.h"

namespace Cygnet {

struct BlendProg: public GlProg<Shader::Blend> {
	void draw(GLuint tex)
	{
		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glCheck();
	}
};

struct ChunkProg: public GlProg<Shader::Chunk> {
	void draw(
		std::span<Renderer::DrawChunk> drawChunks, const Mat3gf &cam,
		GLuint atlasTex, SwanCommon::Vec2 atlasTexSize)
	{
		if (drawChunks.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform2ui(
			shader.uniTileAtlasSize,
			atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTex);
		glUniform1i(shader.uniTileAtlas, 0);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(shader.uniTiles, 1);
		for (auto &[pos, chunk]: drawChunks) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, chunk.tex);
			glDrawArrays(
				GL_TRIANGLES, 0,
				6 * SwanCommon::CHUNK_WIDTH * SwanCommon::CHUNK_HEIGHT);
			glCheck();
		}
	}
};

struct ChunkFluidProg: public GlProg<Shader::ChunkFluid> {
	void draw(
		std::span<Renderer::DrawChunkFluid> drawChunkFluids, const Mat3gf &cam,
		GLuint fluidsTex)
	{
		if (drawChunkFluids.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_1D, fluidsTex);
		glUniform1i(shader.uniFluids, 0);
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(shader.uniFluidGrid, 1);
		for (auto &[pos, fluid]: drawChunkFluids) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, fluid.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct ChunkShadowProg: public GlProg<Shader::ChunkShadow> {
	void draw(std::span<Renderer::DrawChunkShadow> drawChunkShadows, const Mat3gf &cam)
	{
		if (drawChunkShadows.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto &[pos, shadow]: drawChunkShadows) {
			glUniform2f(shader.uniPos, pos.x, pos.y);
			glBindTexture(GL_TEXTURE_2D, shadow.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct RectProg: public GlProg<Shader::Rect> {
	void draw(std::span<Renderer::DrawRect> drawRects, const Mat3gf &cam)
	{
		if (drawRects.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		for (auto &rect: drawRects) {
			glUniform2f(shader.uniPos, rect.pos.x, rect.pos.y);
			glUniform2f(shader.uniSize, rect.size.x, rect.size.y);
			glUniform4f(
				shader.uniOutline, rect.outline.r, rect.outline.g,
				rect.outline.b, rect.outline.a);
			glUniform4f(
				shader.uniFill, rect.fill.r, rect.fill.g,
				rect.fill.b, rect.fill.a);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}

	// I should probably eventually make a bespoke particle renderer,
	// but this works for now
	void drawParticles(std::span<Renderer::DrawParticle> drawParticles, const Mat3gf &cam)
	{
		if (drawParticles.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		auto prevColor = drawParticles.front().color;
		glUniform4f(
			shader.uniOutline, prevColor.r, prevColor.g,
			prevColor.b, prevColor.a);
		glUniform4f(
			shader.uniFill, prevColor.r, prevColor.g,
			prevColor.b, prevColor.a);

		auto prevSize = drawParticles.front().size;
		glUniform2f(shader.uniSize, prevSize.x, prevSize.y);

		for (auto &particle: drawParticles) {
			if (particle.color != prevColor) {
				prevColor = particle.color;
				glUniform4f(
					shader.uniOutline, prevColor.r, prevColor.g,
					prevColor.b, prevColor.a);
				glUniform4f(
					shader.uniFill, prevColor.r, prevColor.g,
					prevColor.b, prevColor.a);
			}

			if (particle.size != prevSize) {
				prevSize = particle.size;
				glUniform2f(shader.uniSize, prevSize.x, prevSize.y);
			}

			glUniform2f(shader.uniPos, particle.pos.x, particle.pos.y);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct SpriteProg: public GlProg<Shader::Sprite> {
	void draw(std::span<Renderer::DrawSprite> drawSprites, const Mat3gf &cam)
	{
		if (drawSprites.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (auto &[mat, sprite, frame]: drawSprites) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, mat.data());
			glUniform2f(shader.uniFrameSize, sprite.size.x, sprite.size.y);
			glUniform2f(shader.uniFrameInfo, sprite.frameCount, frame);
			glBindTexture(GL_TEXTURE_2D, sprite.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct TextProg: public GlProg<Shader::Text> {
	void draw(
		std::span<Renderer::TextSegment> drawTexts,
		std::span<TextCache::RenderedCodepoint> textBuffer,
		const Mat3gf &cam, float scale)
	{
		if (drawTexts.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glUniform1i(shader.uniTextAtlas, 0);
		glUniform1f(shader.uniTextScale, scale);
		glCheck();

		TextAtlas *prevAtlas = nullptr;

		for (const auto &segment: drawTexts) {
			glUniformMatrix3fv(
				shader.uniTransform, 1, GL_TRUE,
				segment.drawText.transform.data());
			glCheck();

			glUniform4f(
				shader.uniColor,
				segment.drawText.color.r, segment.drawText.color.g,
				segment.drawText.color.b, segment.drawText.color.a);
			glCheck();

			if (&segment.atlas != prevAtlas) {
				glUniform2ui(
					shader.uniTextAtlasSize,
					segment.atlas.sideLength * segment.atlas.charWidth,
					segment.atlas.sideLength * segment.atlas.charHeight);
				glCheck();

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, segment.atlas.tex);
				glCheck();

				prevAtlas = &segment.atlas;
			}

			int x = 0;
			for (size_t i = segment.start; i < segment.end; ++i) {
				const auto &rendered = textBuffer[i];

				glUniform2ui(
					shader.uniCharPosition,
					rendered.textureX, rendered.textureY);
				glCheck();

				glUniform2ui(
					shader.uniCharSize,
					rendered.width, segment.atlas.charHeight);
				glCheck();

				x += rendered.x;

				glUniform2i(
					shader.uniPositionOffset,
					x, rendered.y);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
				glCheck();

				x += rendered.width + 1;
			}
		}
	}
};

struct TileProg: public GlProg<Shader::Tile> {
	void draw(
		std::span<Renderer::DrawTile> drawTiles, const Mat3gf &cam,
		GLuint atlasTex, SwanCommon::Vec2 atlasTexSize)
	{
		if (drawTiles.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform2ui(shader.uniTileAtlasSize, atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTex);
		glUniform1i(shader.uniTileAtlas, 0);
		glCheck();

		for (auto &[mat, id, brightness]: drawTiles) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, mat.data());
			glUniform1ui(shader.uniTileID, id);
			glUniform1f(shader.uniBrightness, brightness);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glCheck();
		}
	}
};

struct RendererState {
	BlendProg blendProg{};
	ChunkProg chunkProg{};
	ChunkFluidProg chunkFluidProg{};
	ChunkShadowProg chunkShadowProg{};
	RectProg rectProg{};
	SpriteProg spriteProg{};
	TextProg textProg{};
	TileProg tileProg{};

	SwanCommon::Vec2i screenSize;
	GLuint offscreenFramebuffer = 0;
	GLuint offscreenTex = 0;
	GLuint atlasTex = 0;
	GLuint fluidAtlasTex = 0;
	SwanCommon::Vec2 atlasTexSize;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>())
{
	glGenTextures(1, &state_->atlasTex);
	glCheck();

	glGenTextures(1, &state_->fluidAtlasTex);
	glCheck();

	glGenTextures(1, &state_->offscreenTex);
	glCheck();
}

Renderer::~Renderer()
{
	glDeleteFramebuffers(1, &state_->offscreenFramebuffer);
	glDeleteTextures(1, &state_->offscreenFramebuffer);
	glDeleteTextures(1, &state_->atlasTex);
	glCheck();
}

void Renderer::render(const RenderCamera &cam)
{
	Mat3gf camMat;
	camMat.translate(-cam.pos);

	if (cam.size.y > cam.size.x) {
		float ratio = (float)cam.size.y / (float)cam.size.x;
		winScale_ = {1 / ratio, 1};
		camMat.scale({cam.zoom * ratio, -cam.zoom});
	}
	else {
		float ratio = (float)cam.size.x / (float)cam.size.y;
		winScale_ = {1, 1 / ratio};
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

	for (int i = 0; i <= (int)RenderLayer::MAX; ++i) {
		renderLayer(RenderLayer(i), camMat);
	}

	textBuffer_.clear();
}

void Renderer::renderLayer(RenderLayer layer, Mat3gf camMat)
{
	int idx = (int)layer;

	glBindFramebuffer(GL_FRAMEBUFFER, state_->offscreenFramebuffer);
	glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			state_->offscreenTex, 0);
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);
	glCheck();

	state_->chunkProg.draw(drawChunks_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
	drawChunks_[idx].clear();

	state_->tileProg.draw(drawTiles_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
	drawTiles_[idx].clear();

	state_->spriteProg.draw(drawSprites_[idx], camMat);
	drawSprites_[idx].clear();

	state_->chunkFluidProg.draw(drawChunkFluids_[idx], camMat, state_->fluidAtlasTex);
	drawChunkFluids_[idx].clear();

	state_->rectProg.drawParticles(drawParticles_[idx], camMat);
	drawParticles_[idx].clear();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	state_->blendProg.draw(state_->offscreenTex);

	state_->chunkShadowProg.draw(drawChunkShadows_[idx], camMat);
	drawChunkShadows_[idx].clear();

	state_->rectProg.draw(drawRects_[idx], camMat);
	drawRects_[idx].clear();

	state_->textProg.draw(drawTexts_[idx], textBuffer_, camMat, 1.0 / 128);
	drawTexts_[idx].clear();

	glCheck();
}

void Renderer::renderUI(const RenderCamera &cam)
{
	Mat3gf camMat;

	camMat.translate(-cam.pos);

	if (cam.size.y > cam.size.x) {
		float ratio = (float)cam.size.y / (float)cam.size.x;
		camMat.scale({cam.zoom * ratio, -cam.zoom});
	}
	else {
		float ratio = (float)cam.size.x / (float)cam.size.y;
		camMat.scale({cam.zoom, -cam.zoom * ratio});
	}

	SwanCommon::Vec2 scale = winScale_ / cam.zoom;

	for (int i = 0; i <= (int)RenderLayer::MAX; ++i) {
		renderUILayer(RenderLayer(i), scale, camMat);
	}

	textUIBuffer_.clear();
}

void Renderer::renderUILayer(RenderLayer layer, SwanCommon::Vec2 scale, Mat3gf camMat)
{
	int idx = (int)layer;

	auto applyAnchor = [&](Anchor anchor, Mat3gf &mat, SwanCommon::Vec2 size) {
		switch (anchor) {
		case Anchor::CENTER:
			mat.translate(size * -0.5f);
			break;

		case Anchor::LEFT:
			mat.translate({-scale.x, size.y * -0.5f});
			break;

		case Anchor::RIGHT:
			mat.translate({scale.x - size.x, size.y * -0.5f});
			break;

		case Anchor::TOP:
			mat.translate({size.x * -0.5f, -scale.y});
			break;

		case Anchor::BOTTOM:
			mat.translate({size.x * -0.5f, scale.y - size.y});
			break;

		case Anchor::TOP_LEFT:
			mat.translate({-scale.x, -scale.y});
			break;

		case Anchor::TOP_RIGHT:
			mat.translate({scale.x - size.x, -scale.y});
			break;

		case Anchor::BOTTOM_LEFT:
			mat.translate({-scale.x, scale.y - size.y});
			break;

		case Anchor::BOTTOM_RIGHT:
			mat.translate({scale.x - size.x, scale.y - size.y});
			break;
		}
	};

	assert(drawUISprites_[idx].size() == drawUISpritesAnchors_[idx].size());
	for (size_t i = 0; i < drawUISprites_[idx].size(); ++i) {
		auto &ds = drawUISprites_[idx][i];
		auto anchor = drawUISpritesAnchors_[idx][i];
		applyAnchor(anchor, ds.transform, ds.sprite.size);
	}
	drawUISpritesAnchors_[idx].clear();

	assert(drawUITiles_[idx].size() == drawUITilesAnchors_[idx].size());
	for (size_t i = 0; i < drawUITiles_[idx].size(); ++i) {
		auto &dt = drawUITiles_[idx][i];
		auto anchor = drawUITilesAnchors_[idx][i];
		applyAnchor(anchor, dt.transform, {1, 1});
	}
	drawUITilesAnchors_[idx].clear();

	assert(drawUITexts_[idx].size() == drawUITextsAnchors_[idx].size());
	for (size_t i = 0; i < drawUITexts_[idx].size(); ++i) {
		auto &dt = drawUITexts_[idx][i];
		auto anchor = drawUITextsAnchors_[idx][i];
		applyAnchor(anchor, dt.drawText.transform, dt.size);
	}
	drawUITextsAnchors_[idx].clear();

	state_->spriteProg.draw(drawUISprites_[idx], camMat);
	drawUISprites_[idx].clear();

	state_->tileProg.draw(
		drawUITiles_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
	drawUITiles_[idx].clear();

	state_->textProg.draw(
		drawUITexts_[idx], textUIBuffer_, camMat, 1.0 / 64);
	drawUITexts_[idx].clear();

	glCheck();
}

void Renderer::uploadFluidAtlas(const void *data)
{
	glBindTexture(GL_TEXTURE_1D, state_->fluidAtlasTex);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();
}

void Renderer::uploadTileAtlas(const void *data, int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();

	state_->atlasTexSize = {
		(float)(int)(width / SwanCommon::TILE_SIZE),
		(float)(int)(height / SwanCommon::TILE_SIZE)};
}

void Renderer::modifyTile(TileID id, const void *data)
{
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
	TileID tiles[CHUNK_SIZE])
{
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

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_R16UI,
		SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, tiles);
	glCheck();

	return chunk;
}

void Renderer::modifyChunk(RenderChunk chunk, SwanCommon::Vec2i pos, TileID id)
{
	assert(chunk.tex != ~(GLuint)0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, chunk.tex);
	glCheck();

	glTexSubImage2D(
		GL_TEXTURE_2D, 0, pos.x, pos.y, 1, 1,
		GL_RED_INTEGER, GL_UNSIGNED_SHORT, &id);
	glCheck();
}

void Renderer::destroyChunk(RenderChunk chunk)
{
	assert(chunk.tex != ~(GLuint)0);
	glDeleteTextures(1, &chunk.tex);
	glCheck();
}

RenderChunkFluid Renderer::createChunkFluid(
	uint8_t data[FLUID_CHUNK_SIZE])
{
	RenderChunkFluid fluid;

	glGenTextures(1, &fluid.tex);
	glCheck();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fluid.tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheck();

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_R8UI,
		SwanCommon::CHUNK_WIDTH * SwanCommon::FLUID_RESOLUTION,
		SwanCommon::CHUNK_HEIGHT * SwanCommon::FLUID_RESOLUTION,
		0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
	glCheck();

	return fluid;
}

void Renderer::modifyChunkFluid(
	RenderChunkFluid fluid,
	uint8_t data[FLUID_CHUNK_SIZE])
{
	assert(fluid.tex != ~(GLuint)0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fluid.tex);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_R8UI,
		SwanCommon::CHUNK_WIDTH * SwanCommon::FLUID_RESOLUTION,
		SwanCommon::CHUNK_HEIGHT * SwanCommon::FLUID_RESOLUTION,
		0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, data);
	glCheck();
}

void Renderer::destroyChunkFluid(RenderChunkFluid fluid)
{
	assert(fluid.tex != ~(GLuint)0);
	glDeleteTextures(1, &fluid.tex);
	glCheck();
}

RenderChunkShadow Renderer::createChunkShadow(
	uint8_t data[CHUNK_SIZE])
{
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
	uint8_t data[CHUNK_SIZE])
{
	assert(shadow.tex != ~(GLuint)0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadow.tex);

	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RED,
		SwanCommon::CHUNK_WIDTH, SwanCommon::CHUNK_HEIGHT,
		0, GL_RED, GL_UNSIGNED_BYTE, data);
	glCheck();
}

void Renderer::destroyChunkShadow(RenderChunkShadow shadow)
{
	assert(shadow.tex != ~(GLuint)0);
	glDeleteTextures(1, &shadow.tex);
	glCheck();
}

RenderSprite Renderer::createSprite(void *data, int width, int height, int fh, int repeatFrom)
{
	RenderSprite sprite;

	sprite.size = {
		(float)width / SwanCommon::TILE_SIZE,
		(float)fh / SwanCommon::TILE_SIZE};
	sprite.frameCount = height / fh;
	sprite.repeatFrom = repeatFrom;
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

void Renderer::destroySprite(RenderSprite sprite)
{
	assert(sprite.tex != ~(GLuint)0);
	glDeleteTextures(1, &sprite.tex);
	glCheck();
}

}
