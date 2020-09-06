#include "RenderQueue.h"

#include "glutil.h"

namespace Cygnet {

static const GLfloat texCoords[] = {
	0.0f,      0.0f,      // tex 0: top left
	0.0f,      1.0f,      // tex 1: bottom left
	1.0f,      1.0f,      // tex 2: bottom right
	1.0f,      0.0f,      // tex 3: top right
};

static const GLushort indexes[] = {
	0, 1, 2, // top left -> bottom left -> bottom right
	2, 3, 0, // bottom right -> top right -> top left
};

void RenderQueue::draw() {
	glUniform1i(locs_.tex, 0);
	glVertexAttribPointer(locs_.texCoord, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), texCoords);
	glEnableVertexAttribArray(locs_.texCoord);
	glCheck();

	glUniformMatrix3fv(locs_.transform, 1, GL_TRUE, mat_);
	glCheck();

	glActiveTexture(GL_TEXTURE0);

	for (auto &entry: queue_) {
		float w = entry.w * entry.sx;
		float h = entry.h * entry.sy;
		float x = entry.x;
		float y = entry.y;
		GLfloat vertexes[] = {
			x,     y,     // top left
			x,     y + h, // bottom left
			x + w, y + h, // bottom right
			x + w, y,     // top right
		};

		glVertexAttribPointer(locs_.position, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), vertexes);
		glEnableVertexAttribArray(locs_.position);
		glCheck();

		glBindTexture(GL_TEXTURE_2D, entry.tex);
		glCheck();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indexes);
	}

	queue_.clear();
}

}
