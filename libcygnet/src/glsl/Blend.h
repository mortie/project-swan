#include "Blend.glsl.h"

#include "GlWrappers.h"

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

}
