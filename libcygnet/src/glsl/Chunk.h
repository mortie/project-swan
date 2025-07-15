#include "Chunk.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

struct ChunkProg: public GlProg<Shader::Chunk> {
	void draw(
		std::span<Renderer::DrawChunk> drawChunks, const Mat3gf &cam,
		GLuint atlasTex, Swan::Vec2 atlasTexSize,
		GLuint mapTex, float mapTexSize)
	{
		if (drawChunks.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, atlasTex);
		glUniform1i(shader.uniTileAtlas, 0);
		glCheck();

		glUniform2ui(shader.uniTileAtlasSize, atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mapTex);
		glUniform1i(shader.uniTileMap, 1);
		glCheck();

		glUniform1f(shader.uniTileMapSize, mapTexSize);
		glCheck();

		glActiveTexture(GL_TEXTURE2);
		glUniform1i(shader.uniTiles, 2);
		glCheck();

		for (const auto &dc: drawChunks) {
			glUniform2f(shader.uniPos, dc.pos.x, dc.pos.y);
			glBindTexture(GL_TEXTURE_2D, dc.chunk.tex);
			glDrawArrays(
				GL_TRIANGLES, 0,
				6 * Swan::CHUNK_WIDTH * Swan::CHUNK_HEIGHT);
			glCheck();
		}
	}
};

}
