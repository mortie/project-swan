#include "AlphaBlend.glsl.h"

#include "GlWrappers.h"

namespace Cygnet {

struct AlphaBlendProg: public GlProg<Shader::AlphaBlend> {
	void draw(GLuint tex, float alpha)
	{
		glUseProgram(id());
		glCheck();

		glUniform1i(shader.uniTex, 0);
		glCheck();
		glUniform1f(shader.uniAlpha, alpha);
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glCheck();
	}
};

}
