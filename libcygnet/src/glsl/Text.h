#include "Text.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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
			auto transform = Mat3gf{}
				.scale({segment.drawText.scale, segment.drawText.scale})
				.translate(segment.drawText.pos);

			glUniformMatrix3fv(
				shader.uniTransform, 1, GL_TRUE,
				transform.data());
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

}
