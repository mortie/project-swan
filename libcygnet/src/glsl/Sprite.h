#include "Sprite.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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

}
