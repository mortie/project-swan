#include "Particle.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

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

}
