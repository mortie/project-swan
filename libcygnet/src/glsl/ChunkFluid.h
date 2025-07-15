#include "ChunkFluid.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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

}
