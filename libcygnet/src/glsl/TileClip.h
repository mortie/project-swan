#include "TileClip.glsl.h"

#include <span>

#include "GlWrappers.h"
#include "Renderer.h"

namespace Cygnet {

struct TileClipProg: public GlProg<Shader::TileClip> {
	static constexpr int TEXTURE_COUNT =
		int(TileClip::TOP_LEFT | TileClip::TOP_RIGHT);

	TileClipProg()
	{
		glActiveTexture(GL_TEXTURE0);

		unsigned char buf[Swan::TILE_SIZE][Swan::TILE_SIZE];

		glGenTextures(TEXTURE_COUNT, textures);
		for (int i = 0; i < TEXTURE_COUNT; ++i) {
			auto clip = TileClip(i + 1);
			memset(buf, 0, sizeof(buf));

			if (clip & TileClip::TOP_LEFT) {
				buf[0][0] = 1;
				buf[0][1] = 1;
				buf[1][0] = 1;
			}

			if (clip & TileClip::TOP_RIGHT) {
				buf[0][Swan::TILE_SIZE - 1] = 1;
				buf[0][Swan::TILE_SIZE - 2] = 1;
				buf[1][Swan::TILE_SIZE - 1] = 1;
			}

			glBindTexture(GL_TEXTURE_2D, textures[i]);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_R8UI,
				Swan::TILE_SIZE, Swan::TILE_SIZE,
				0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, buf);
			glCheck();
		}
	}

	~TileClipProg()
	{
		glDeleteTextures(TEXTURE_COUNT, textures);
	}

	void draw(
		std::span<Renderer::DrawTileClip> drawClips, const Mat3gf &cam)
	{
		if (drawClips.size() == 0) {
			return;
		}

		glUseProgram(id());
		glCheck();

		glUniformMatrix3fv(shader.uniCamera, 1, GL_TRUE, cam.data());
		glCheck();

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(shader.uniTex, 0);

		TileClip prevClip = TileClip::NONE;
		for (auto &dc: drawClips) {
			if (dc.clip != prevClip) {
				glBindTexture(GL_TEXTURE_2D, textures[int(dc.clip) - 1]);
				glCheck();
			}

			glUniform2f(shader.uniPos, dc.pos.x, dc.pos.y);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			glCheck();
		}
	}

	GLuint textures[int(TileClip::TOP_LEFT | TileClip::TOP_RIGHT)];
};

}
