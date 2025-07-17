#include "Renderer.h"

#include <cassert>
#include <iostream>
#include <stdio.h>
#include <swan/constants.h>
#include <string.h>
#include <span>

#include "GlWrappers.h"
#include "util.h"

#include "glsl/Blend.h"
#include "glsl/Chunk.h"
#include "glsl/ChunkFluid.h"
#include "glsl/ChunkShadow.h"
#include "glsl/Particle.h"
#include "glsl/Rect.h"
#include "glsl/Sprite.h"
#include "glsl/Text.h"
#include "glsl/Tile.h"
#include "glsl/TileClip.h"

namespace Cygnet {

struct RendererState {
	BlendProg blendProg{};
	ChunkProg chunkProg{};
	ChunkFluidProg chunkFluidProg{};
	ChunkShadowProg chunkShadowProg{};
	ParticleProg particleProg{};
	RectProg rectProg{};
	SpriteProg spriteProg{};
	TextProg textProg{};
	TileProg tileProg{};
	TileClipProg tileClipProg{};

	Swan::Vec2i screenSize;
	GLuint offscreenFramebuffer = 0;
	GLuint offscreenTex = 0;
	GLuint offscreenStencilTex = 0;
	GLuint tileAtlasTex = 0;
	Swan::Vec2 tileAtlasTexSize;
	GLuint tileMapTex = 0;
	float tileMapTexSize = 0;
	GLuint fluidAtlasTex = 0;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>())
{
	glGenTextures(1, &state_->tileAtlasTex);
	glCheck();

	glGenTextures(1, &state_->tileMapTex);
	glCheck();

	glGenTextures(1, &state_->fluidAtlasTex);
	glCheck();

	glGenTextures(1, &state_->offscreenTex);
	glCheck();

	glGenTextures(1, &state_->offscreenStencilTex);
	glCheck();
}

Renderer::~Renderer()
{
	glDeleteFramebuffers(1, &state_->offscreenFramebuffer);
	glDeleteTextures(1, &state_->offscreenStencilTex);
	glDeleteTextures(1, &state_->offscreenTex);
	glDeleteTextures(1, &state_->tileAtlasTex);
	glDeleteTextures(1, &state_->tileMapTex);
	glDeleteTextures(1, &state_->fluidAtlasTex);
	glCheck();
}

void Renderer::update(float dt)
{
	constexpr float GRAVITY = 20;

	for (int layer = 0; layer < LAYER_COUNT; ++layer) {
		auto &particles = spawnedParticles_[layer];
		auto &metas = spawnedParticleMetas_[layer];

		assert(particles.size() == metas.size());
		for (size_t i = 0; i < particles.size(); ++i) {
			auto &meta = metas[i];
			meta.lifetime -= dt;
			if (meta.lifetime <= 0) {
				particles[i] = particles.back();
				particles.pop_back();
				metas[i] = metas.back();
				metas.pop_back();
				continue;
			}

			meta.vel.y += GRAVITY * meta.weight * dt;
			particles[i].pos += meta.vel * dt;
		}
	}
}

void Renderer::clear()
{
	for (int idx = 0; idx <= (int)RenderLayer::MAX; ++idx) {
		drawTiles_[idx].clear();
		drawTileSprites_[idx].clear();
		drawSprites_[idx].clear();
		drawParticles_[idx].clear();
		drawRects_[idx].clear();
		drawTexts_[idx].clear();

		drawUIGrids_[idx].clear();
		drawUISprites_[idx].clear();
		drawUITiles_[idx].clear();
		drawUIRects_[idx].clear();
		drawUITexts_[idx].clear();
	}

	drawChunks_.clear();
	drawChunkFluids_.clear();
	drawChunkShadows_.clear();
	drawTileClips_.clear();
	textBuffer_.clear();
	textUIBuffer_.clear();
}

void Renderer::render(const RenderCamera &cam, RenderProps props)
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

	if (props.vflip) {
		camMat.scale({1, -1});
	}

	GLint screenFBO;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &screenFBO);
	glCheck();

	if (state_->screenSize != cam.size) {
		state_->screenSize = cam.size;

		// Generate offscreen texture
		glBindTexture(GL_TEXTURE_2D, state_->offscreenTex);
		glCheck();
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_RGBA, cam.size.x, cam.size.y, 0,
			GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glCheck();

		// Generate offscreen stencil texture
		glBindTexture(GL_TEXTURE_2D, state_->offscreenStencilTex);
		glCheck();
		glTexImage2D(
			GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, cam.size.x, cam.size.y, 0,
			GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
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
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
			state_->offscreenStencilTex, 0);
		glCheck();
	}

	// Set up and clear the offscreen frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, state_->offscreenFramebuffer);
	glFramebufferTexture2D(
		GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
		state_->offscreenTex, 0);
	glClearColor(
		backgroundColor_.r, backgroundColor_.g,
		backgroundColor_.b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glCheck();

	// Render background layers
	for (int i = 0; i < (int)RenderLayer::NORMAL; ++i) {
		renderLayer(RenderLayer(i), camMat);
	}

	// Render Fluids
	state_->chunkFluidProg.draw(drawChunkFluids_, camMat, state_->fluidAtlasTex, 1);

	// Render to the stencil buffer for corner clipping
	glColorMask(false, false, false, false);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glStencilFunc(GL_EQUAL, 0, 0x01);
	state_->tileClipProg.draw(drawTileClips_, camMat);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glColorMask(true, true, true, true);

	// Render the world, with the stencil test active to clip corners
	state_->chunkProg.draw(
		drawChunks_, camMat, state_->tileAtlasTex, state_->tileAtlasTexSize,
		state_->tileMapTex, state_->tileMapTexSize);
	glDisable(GL_STENCIL_TEST);

	// Render the normal layer
	renderLayer(RenderLayer::NORMAL, camMat);

	// Render another layer of fluids, above the normal layer
	state_->chunkFluidProg.draw(drawChunkFluids_, camMat, state_->fluidAtlasTex, 0);

	// Render the foreground layers
	for (int i = (int)RenderLayer::NORMAL + 1; i <= (int)RenderLayer::MAX; ++i) {
		renderLayer(RenderLayer(i), camMat);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, screenFBO);
	state_->blendProg.draw(state_->offscreenTex, gamma_);
	glCheck();

	state_->chunkShadowProg.draw(drawChunkShadows_, camMat, gamma_);
	glCheck();

	textBuffer_.clear();
}

void Renderer::renderLayer(RenderLayer layer, Mat3gf camMat)
{
	int idx = (int)layer;

	state_->tileProg.draw(
		drawTiles_[idx], camMat, state_->tileAtlasTex, state_->tileAtlasTexSize,
		state_->tileMapTex, state_->tileMapTexSize);
	state_->spriteProg.draw(drawTileSprites_[idx], camMat);
	state_->spriteProg.draw(drawSprites_[idx], camMat);
	state_->particleProg.draw(drawParticles_[idx], camMat);

	// Use the stencil buffer to ensure that spawned particles don't
	// draw over each other.
	// Note: this uses the same stencil buffer across all layers without clearing.
	if (!spawnedParticles_[idx].empty()) {
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glStencilFunc(GL_EQUAL, 0, 0x01);

		state_->particleProg.draw(spawnedParticles_[idx], camMat);

		glDisable(GL_STENCIL_TEST);
	}

	state_->rectProg.draw(drawRects_[idx], camMat);
	state_->textProg.draw(drawTexts_[idx], textBuffer_, camMat, 1.0 / 128);

	glCheck();
}

void Renderer::renderUI(const RenderCamera &cam, RenderProps props)
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

	Swan::Vec2 scale = winScale_ / cam.zoom;
	uiViewStack_.resize(1);
	uiViewStack_[0].size = scale;

	if (props.vflip){ 
		camMat.scale({1, -1});
	}

	for (int i = 0; i <= (int)RenderLayer::MAX; ++i) {
		renderUILayer(RenderLayer(i), camMat);
	}
}

void Renderer::renderUILayer(RenderLayer layer, Mat3gf camMat)
{
	int idx = (int)layer;

	state_->spriteProg.drawGrids(drawUIGrids_[idx], camMat);
	state_->spriteProg.draw(drawUISprites_[idx], camMat);
	state_->tileProg.draw(
		drawUITiles_[idx], camMat, state_->tileAtlasTex, state_->tileAtlasTexSize,
		state_->tileMapTex, state_->tileMapTexSize);
	state_->rectProg.draw(drawUIRects_[idx], camMat);
	state_->textProg.draw(
		drawUITexts_[idx], textUIBuffer_, camMat, 1.0 / 64);
	glCheck();
}

void Renderer::applyAnchor(Anchor anchor, Mat3gf &mat, Swan::Vec2 size)
{
	auto &view = uiViewStack_.back();
	mat.translate(view.pos);

	switch (anchor) {
	case Anchor::CENTER:
		mat.translate(size * -0.5f);
		break;

	case Anchor::LEFT:
		mat.translate({-view.size.x, size.y * -0.5f});
		break;

	case Anchor::RIGHT:
		mat.translate({view.size.x - size.x, size.y * -0.5f});
		break;

	case Anchor::TOP:
		mat.translate({size.x * -0.5f, -view.size.y});
		break;

	case Anchor::BOTTOM:
		mat.translate({size.x * -0.5f, view.size.y - size.y});
		break;

	case Anchor::TOP_LEFT:
		mat.translate({-view.size.x, -view.size.y});
		break;

	case Anchor::TOP_RIGHT:
		mat.translate({view.size.x - size.x, -view.size.y});
		break;

	case Anchor::BOTTOM_LEFT:
		mat.translate({-view.size.x, view.size.y - size.y});
		break;

	case Anchor::BOTTOM_RIGHT:
		mat.translate({view.size.x - size.x, view.size.y - size.y});
		break;
	}
};

void Renderer::applyAnchor(Anchor anchor, Swan::Vec2 &pos, Swan::Vec2 size)
{
	auto &view = uiViewStack_.back();
	pos += view.pos;

	switch (anchor) {
	case Anchor::CENTER:
		pos += size * -0.5f;
		break;

	case Anchor::LEFT:
		pos += {-view.size.x, size.y * -0.5f};
		break;

	case Anchor::RIGHT:
		pos += {view.size.x - size.x, size.y * -0.5f};
		break;

	case Anchor::TOP:
		pos += {size.x * -0.5f, -view.size.y};
		break;

	case Anchor::BOTTOM:
		pos += {size.x * -0.5f, view.size.y - size.y};
		break;

	case Anchor::TOP_LEFT:
		pos += {-view.size.x, -view.size.y};
		break;

	case Anchor::TOP_RIGHT:
		pos += {view.size.x - size.x, -view.size.y};
		break;

	case Anchor::BOTTOM_LEFT:
		pos += {-view.size.x, view.size.y - size.y};
		break;

	case Anchor::BOTTOM_RIGHT:
		pos += {view.size.x - size.x, view.size.y - size.y};
		break;
	}
};

Renderer::Rect Renderer::pushUIView(Rect rect, Anchor anchor)
{
	auto &view = uiViewStack_.back();
	rect.size /= 2;
	rect.pos += view.pos;
	switch (anchor) {
	case Anchor::CENTER:
		break;

	case Anchor::LEFT:
		rect.pos += {-view.size.x + rect.size.x, 0};
		break;

	case Anchor::RIGHT:
		rect.pos += {view.size.x - rect.size.x, 0};
		break;

	case Anchor::TOP:
		rect.pos += {0, -view.size.y + rect.size.y};
		break;

	case Anchor::BOTTOM:
		rect.pos += {0, view.size.y - rect.size.y};
		break;

	case Anchor::TOP_LEFT:
		rect.pos += {-view.size.x + rect.size.x, -view.size.y + rect.size.y};
		break;

	case Anchor::TOP_RIGHT:
		rect.pos += {view.size.x - rect.size.x, -view.size.y + rect.size.y};
		break;

	case Anchor::BOTTOM_LEFT:
		rect.pos += {-view.size.x + rect.size.x, view.size.y - rect.size.y};
		break;

	case Anchor::BOTTOM_RIGHT:
		rect.pos += {view.size.x - rect.size.x, view.size.y - rect.size.y};
		break;
	}

	uiViewStack_.push_back(rect);
	return rect;
}

void Renderer::popUIView()
{
	if (uiViewStack_.size() <= 1) {
		std::cerr << "Cygnet: View push/pop mismatch\n";
		return;
	}

	uiViewStack_.pop_back();
}

bool Renderer::assertUIViewStackEmpty()
{
	if (uiViewStack_.size() > 1) {
		uiViewStack_.resize(1);
		return false;
	}

	return true;
}

void Renderer::uploadFluidAtlas(const void *data)
{
	glBindTexture(GL_TEXTURE_2D, state_->fluidAtlasTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();
}

void Renderer::uploadTileAtlas(const void *data, int width, int height)
{
	glBindTexture(GL_TEXTURE_2D, state_->tileAtlasTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();

	state_->tileAtlasTexSize = {
		(float)(int)(width / Swan::TILE_SIZE),
		(float)(int)(height / Swan::TILE_SIZE)};
}

void Renderer::uploadTileMap(std::span<uint16_t> tiles)
{
	glBindTexture(GL_TEXTURE_2D, state_->tileMapTex);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_R16UI, tiles.size(), 1,
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, tiles.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glCheck();

	state_->tileMapTexSize = tiles.size();
}

void Renderer::modifyTile(TileID id, uint16_t newAssetID)
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state_->tileMapTex);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, id, 0, 1, 1,
		GL_RED_INTEGER, GL_UNSIGNED_SHORT, &newAssetID);
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
		Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT,
		0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, tiles);
	glCheck();

	return chunk;
}

void Renderer::modifyChunk(RenderChunk chunk, Swan::Vec2i pos, TileID id)
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
		Swan::CHUNK_WIDTH * Swan::FLUID_RESOLUTION,
		Swan::CHUNK_HEIGHT * Swan::FLUID_RESOLUTION,
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
		Swan::CHUNK_WIDTH * Swan::FLUID_RESOLUTION,
		Swan::CHUNK_HEIGHT * Swan::FLUID_RESOLUTION,
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
		Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT,
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
		Swan::CHUNK_WIDTH, Swan::CHUNK_HEIGHT,
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
		(float)width / Swan::TILE_SIZE,
		(float)fh / Swan::TILE_SIZE};
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
