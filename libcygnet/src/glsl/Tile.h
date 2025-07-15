#include "Tile.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

struct TileProg: public GlProg<Shader::Tile> {
	void draw(
		std::span<Renderer::DrawTile> drawTiles, const Mat3gf &cam,
		GLuint atlasTex, Swan::Vec2 atlasTexSize,
		GLuint mapTex, float mapTexSize)
	{
		if (drawTiles.size() == 0) {
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

		glUniform2f(shader.uniTileAtlasSize, atlasTexSize.x, atlasTexSize.y);
		glCheck();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mapTex);
		glUniform1i(shader.uniTileMap, 1);
		glCheck();

		glUniform1f(shader.uniTileMapSize, mapTexSize);
		glCheck();

		for (const auto &dt: drawTiles) {
			glUniformMatrix3fv(shader.uniTransform, 1, GL_TRUE, dt.transform.data());
			glUniform1ui(shader.uniTileID, dt.id);
			glUniform1f(shader.uniBrightness, dt.brightness);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			glCheck();
		}
	}
};

}
