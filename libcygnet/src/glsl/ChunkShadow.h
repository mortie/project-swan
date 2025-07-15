#include "ChunkShadow.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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

}
