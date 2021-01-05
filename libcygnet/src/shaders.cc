#include "shaders.h"

namespace Cygnet::Shaders {

const char *spriteVx = R"glsl(
	#define TILE_SIZE 32.0

	uniform mat3 camera;
	uniform mat3 transform;
	uniform vec3 frameInfo; // frame height, frame count, frame index
	attribute vec2 vertex;
	varying vec2 v_texCoord;

	void main() {
		// Here, I'm basically treating 1/(TILE_SIZE*16) as half the size of a "pixel".
		// It's just an arbitrary small number, but it works as an offset to make sure
		// neighbouring parts of the atlas don't bleed into the frame we actually
		// want to draw due to (nearest neighbour) interpolation.
		float pixoffset = (1.0 - vertex.y * 2.0) / (frameInfo.x * TILE_SIZE * 16.0);
		v_texCoord = vec2(
			vertex.x,
			(frameInfo.x * frameInfo.z + (frameInfo.x * vertex.y)) /
			(frameInfo.x * frameInfo.y) + pixoffset);

		vec3 pos = camera * transform * vec3(vertex, 1);
		gl_Position = vec4(pos.xy, 0, 1);
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
		gl_Position = vec4(pos.xy, 0, 1);
		v_tileCoord = vec2(vertex.x, vertex.y);
	}
)glsl";

const char *chunkFr = R"glsl(
	precision mediump float;
	#define TILE_SIZE 32.0
	#define CHUNK_WIDTH 64
	#define CHUNK_HEIGHT 64

	varying vec2 v_tileCoord;
	uniform sampler2D tileAtlas;
	uniform vec2 tileAtlasSize;
	uniform sampler2D tiles;

	void main() {
		vec2 tilePos = floor(vec2(v_tileCoord.x, v_tileCoord.y));
		vec4 tileColor = texture2D(tiles, tilePos / vec2(CHUNK_WIDTH, CHUNK_HEIGHT));
		float tileID = floor((tileColor.r * 256.0 + tileColor.a) * 256.0);

		// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
		vec2 offset = v_tileCoord - tilePos;
		vec2 pixoffset = (1.0 - offset * 2.0) / (TILE_SIZE * 16.0);
		vec2 atlasPos = vec2(
			pixoffset.x + tileID + offset.x,
			pixoffset.y + floor(tileID / tileAtlasSize.x) + offset.y);

		gl_FragColor = texture2D(tileAtlas, atlasPos / tileAtlasSize);
	}
)glsl";

}
