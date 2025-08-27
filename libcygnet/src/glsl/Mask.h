#include "Mask.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

struct MaskProg: public GlProg<Shader::Mask> {
	void draw(std::span<Renderer::DrawMask> drawMasks, const Mat3gf &cam)
	{
		if (drawMasks.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(shader.uniTex, 0);
		glCheck();

		for (const auto &mask: drawMasks) {
			glUniform2f(shader.uniPos, mask.pos.x, mask.pos.y);
			glUniform2f(shader.uniSize, mask.mask.size.x, mask.mask.size.y);
			glBindTexture(GL_TEXTURE_2D, mask.mask.tex);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}
};

}
