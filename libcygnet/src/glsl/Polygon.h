#include "Polygon.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"
#include "gl.h"

namespace Cygnet {

struct PolygonProg: public GlProg<Shader::Polygon> {
	void drawTriangleStrip(
		std::span<Renderer::TriangleStripSegment> drawSegments,
		std::span<Swan::Vec2> buffer,
		GLuint vbo, const Mat3gf &cam)
	{
		if (drawSegments.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray(0);

		for (const auto &segment: drawSegments) {
			glUniform4f(
				shader.uniFill, segment.fill.r, segment.fill.g,
				segment.fill.b, segment.fill.a);

			size_t length = segment.end - segment.start;
			auto *data = buffer.data() + segment.start;
			glBufferData(GL_ARRAY_BUFFER, length * sizeof(Swan::Vec2), data, GL_STREAM_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, length);
			glCheck();
		}
	}
};

}
