#include "shaders.h"

namespace Cygnet::Shaders {

const char *spriteVx = R"glsl(
	uniform mat3 camera;
	uniform mat3 transform;
	attribute vec2 vertex;
	attribute vec2 texCoord;
	varying vec2 v_texCoord;

	void main() {
		vec3 pos = camera * transform * vec3(vertex, 1);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
		v_texCoord = texCoord;
	}
)glsl";

const char *spriteFr = R"glsl(
	precision mediump float;
	varying vec2 v_texCoord;
	uniform sampler2D tex;

	void main() {
		gl_FragColor = texture2D(tex, v_texCoord);
	}
)glsl";

const char *chunkVx = R"glsl(
	uniform mat3 camera;
	uniform vec2 pos;
	attribute vec2 vertex;
	varying vec2 v_tileCoord;

	void main() {
		vec3 pos = camera * vec3(pos + vertex, 1);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
		v_tileCoord = vertex;
	}
)glsl";

const char *chunkFr = R"glsl(
	precision mediump float;
	#define TILE_SIZE 32.0
	#define CHUNK_WIDTH 64
	#define CHUNK_HEIGHT 64

	varying vec2 v_tileCoord;
	uniform sampler2D tileTex;
	uniform float tileTexWidth;
	uniform float tileTexHeight;
	uniform int tiles[CHUNK_WIDTH * CHUNK_HEIGHT];

	void main() {
		float tileX = floor(v_tileCoord.x);
		float tileY = floor(-v_tileCoord.y);
		int tileIndex = int(tileY * float(CHUNK_WIDTH) + tileX);
		float tileID = float(tiles[tileIndex]);

		float atlasX = floor(mod(tileID, tileTexWidth)) + v_tileCoord.x - tileX;
		float atlasY = floor(tileID / tileTexWidth) + (-v_tileCoord.y - tileY);

		float x = atlasX / tileTexWidth;
		float y = atlasY / tileTexHeight;

		gl_FragColor = texture2D(tileTex, vec2(x, y));
	}
)glsl";

}
