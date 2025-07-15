#include "Rect.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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

}
