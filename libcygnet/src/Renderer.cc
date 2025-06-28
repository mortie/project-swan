#include "Renderer.h"

#include <cassert>
#include <iostream>
#include <stdio.h>
#include <swan/constants.h>
#include <string.h>
#include <span>

#include "GlWrappers.h"
#include "util.h"

#include "Blend.glsl.h"
#include "Chunk.glsl.h"
#include "ChunkFluid.glsl.h"
#include "ChunkShadow.glsl.h"
#include "Particle.glsl.h"
#include "Rect.glsl.h"
#include "Sprite.glsl.h"
#include "Text.glsl.h"
#include "Tile.glsl.h"

namespace Cygnet {

struct BlendProg: public GlProg<Shader::Blend> {
	void draw(GLuint tex, float gamma)
	{
		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glCheck();
		glUniform1f(shader.uniDesaturate, std::min(gamma - 1.0, 1.0));
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
		GLuint atlasTex, Swan::Vec2 atlasTexSize)
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
		for (const auto &dc: drawChunks) {
			glUniform2f(shader.uniPos, dc.pos.x, dc.pos.y);
			glBindTexture(GL_TEXTURE_2D, dc.chunk.tex);
			glDrawArrays(
				GL_TRIANGLES, 0,
				6 * Swan::CHUNK_WIDTH * Swan::CHUNK_HEIGHT);
			glCheck();
		}
	}
};

struct ChunkFluidProg: public GlProg<Shader::ChunkFluid> {
	void draw(
		std::span<Renderer::DrawChunkFluid> drawChunkFluids, const Mat3gf &cam,
		GLuint fluidsTex, float row)
	{
		if (drawChunkFluids.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fluidsTex);
		glUniform1i(shader.uniFluids, 0);
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform1f(shader.uniRow, row);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		glUniform1i(shader.uniFluidGrid, 1);
		for (const auto &df: drawChunkFluids) {
			glUniform2f(shader.uniPos, df.pos.x, df.pos.y);
			glBindTexture(GL_TEXTURE_2D, df.fluids.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct ChunkShadowProg: public GlProg<Shader::ChunkShadow> {
	void draw(
		std::span<Renderer::DrawChunkShadow> drawChunkShadows,
		const Mat3gf &cam,
		float desaturate)
	{
		if (drawChunkShadows.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniform1f(shader.uniGamma, desaturate);
		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (const auto &dcs: drawChunkShadows) {
			glUniform2f(shader.uniPos, dcs.pos.x, dcs.pos.y);
			glBindTexture(GL_TEXTURE_2D, dcs.shadow.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

struct ParticleProg: public GlProg<Shader::Particle> {
	void draw(std::span<Renderer::DrawParticle> drawParticles, const Mat3gf &cam)
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
			shader.uniFill, prevColor.r, prevColor.g,
			prevColor.b, prevColor.a);

		auto prevSize = drawParticles.front().size;
		glUniform2f(shader.uniSize, prevSize.x, prevSize.y);

		for (const auto &particle: drawParticles) {
			if (particle.color != prevColor) {
				prevColor = particle.color;
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

		for (const auto &rect: drawRects) {
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
};

struct SpriteProg: public GlProg<Shader::Sprite> {
	void draw(std::span<Renderer::DrawSprite> drawSprites, const Mat3gf &cam)
	{
		if (drawSprites.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniform2f(shader.uniTranslate, 0, 0);
		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform1f(shader.uniOpacity, 1.0);
		glCheck();
		float prevOpacity = 1;

		glActiveTexture(GL_TEXTURE0);
		for (const auto &ds: drawSprites) {
			if (ds.opacity != prevOpacity) {
				glUniform1f(shader.uniOpacity, ds.opacity);
				prevOpacity = ds.opacity;
			}

			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, ds.transform.data());
			glUniform2f(shader.uniFrameSize, ds.sprite.size.x, ds.sprite.size.y);
			glUniform2f(shader.uniFrameInfo, ds.sprite.frameCount, ds.frame);
			glBindTexture(GL_TEXTURE_2D, ds.sprite.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}

	void drawGrids(std::span<Renderer::DrawGrid> drawGrids, const Mat3gf &cam)
	{
		if (drawGrids.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glUniform1f(shader.uniOpacity, 1.0);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		for (const auto &dg: drawGrids) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, dg.transform.data());
			glUniform2f(shader.uniFrameSize, dg.sprite.size.x, dg.sprite.size.y);
			glBindTexture(GL_TEXTURE_2D, dg.sprite.tex);

			// Draw top left, top middle, top right (0, 1, 2)
			drawStrip(dg.sprite, dg.w, 0, 0, 1, 2);
			glCheck();

			// Draw center left, center, center right (3, 4, 5)
			for (int y = 0; y < dg.h; ++y) {
				drawStrip(dg.sprite, dg.w, y + 1, 3, 4, 5);
				glCheck();
			}

			// Draw bottom left, bottom middle, bottom right (6, 7, 8)
			drawStrip(dg.sprite, dg.w, dg.h + 1, 6, 7, 8);
			glCheck();
		}
	}

private:
	void drawStrip(const RenderSprite &sprite, int w, int y, int l, int m, int r)
	{
		// Left
		glUniform2f(shader.uniFrameInfo, sprite.frameCount, l);
		glUniform2f(shader.uniTranslate, 0, y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glCheck();

		// Middle
		glUniform2f(shader.uniFrameInfo, sprite.frameCount, m);
		for (int x = 0; x < w; ++x) {
			glUniform2f(shader.uniTranslate, x + 1, y);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}

		// Right
		glUniform2f(shader.uniFrameInfo, sprite.frameCount, r);
		glUniform2f(shader.uniTranslate, w + 1, y);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glCheck();
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
		GLuint atlasTex, Swan::Vec2 atlasTexSize)
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

		for (const auto &dt: drawTiles) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, dt.transform.data());
			glUniform1ui(shader.uniTileID, dt.id);
			glUniform1f(shader.uniBrightness, dt.brightness);
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
	ParticleProg particleProg{};
	RectProg rectProg{};
	SpriteProg spriteProg{};
	TextProg textProg{};
	TileProg tileProg{};

	Swan::Vec2i screenSize;
	GLuint offscreenFramebuffer = 0;
	GLuint offscreenTex = 0;
	GLuint offscreenStencilTex = 0;
	GLuint atlasTex = 0;
	GLuint fluidAtlasTex = 0;
	Swan::Vec2 atlasTexSize;
};

Renderer::Renderer(): state_(std::make_unique<RendererState>())
{
	glGenTextures(1, &state_->atlasTex);
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
	glDeleteTextures(1, &state_->atlasTex);
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
		drawChunks_[idx].clear();
		drawTiles_[idx].clear();
		drawTileSprites_[idx].clear();
		drawSprites_[idx].clear();
		drawChunkFluids_[idx].clear();
		drawParticles_[idx].clear();
		drawRects_[idx].clear();
		drawTexts_[idx].clear();

		drawUIGrids_[idx].clear();
		drawUISprites_[idx].clear();
		drawUITiles_[idx].clear();
		drawUIRects_[idx].clear();
		drawUITexts_[idx].clear();
	}

	drawChunkShadows_.clear();
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

	glBindFramebuffer(GL_FRAMEBUFFER, state_->offscreenFramebuffer);
	glFramebufferTexture2D(
			GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
			state_->offscreenTex, 0);
	glClearColor(
		backgroundColor_.r, backgroundColor_.g,
		backgroundColor_.b, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glCheck();

	for (int i = 0; i <= (int)RenderLayer::MAX; ++i) {
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

	state_->chunkFluidProg.draw(drawChunkFluids_[idx], camMat, state_->fluidAtlasTex, 1);
	state_->chunkProg.draw(drawChunks_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
	state_->tileProg.draw(drawTiles_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
	state_->spriteProg.draw(drawTileSprites_[idx], camMat);
	state_->spriteProg.draw(drawSprites_[idx], camMat);
	state_->particleProg.draw(drawParticles_[idx], camMat);

	// Use the stencil buffer to ensure that spawned particles don't
	// draw over each other.
	// Note: this uses the same stencil buffer across all layers without clearing.
	{
		glEnable(GL_STENCIL_TEST);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
		glStencilFunc(GL_EQUAL, 0, 0x01);

		state_->particleProg.draw(spawnedParticles_[idx], camMat);

		glDisable(GL_STENCIL_TEST);
	}

	state_->chunkFluidProg.draw(drawChunkFluids_[idx], camMat, state_->fluidAtlasTex, 0);
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
		drawUITiles_[idx], camMat, state_->atlasTex, state_->atlasTexSize);
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
	glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glCheck();

	state_->atlasTexSize = {
		(float)(int)(width / Swan::TILE_SIZE),
		(float)(int)(height / Swan::TILE_SIZE)};
}

void Renderer::modifyTile(TileID id, const void *data)
{
	int w = (int)state_->atlasTexSize.x;
	int x = id % w;
	int y = id / w;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, state_->atlasTex);
	glTexSubImage2D(
		GL_TEXTURE_2D, 0, x * Swan::TILE_SIZE, y * Swan::TILE_SIZE,
		Swan::TILE_SIZE, Swan::TILE_SIZE,
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
