#include "shaders.h"

namespace Cygnet::Shaders {

const char *chunkVx = R"glsl(
	in vec2 vertex;
	uniform mat3 camera;
	uniform vec2 pos;
	out vec2 v_tileCoord;

	void main() {
		vec3 p = camera * vec3(pos + vertex, 1);
		gl_Position = vec4(p.xy, 0, 1);
		v_tileCoord = vertex;
	}
)glsl";

const char *chunkFr = R"glsl(
	in vec2 v_tileCoord;
	uniform sampler2D tileAtlas;
	uniform vec2 tileAtlasSize;
	uniform sampler2D tiles;
	out vec4 fragColor;

	void main() {
		vec2 tilePos = floor(vec2(v_tileCoord.x, v_tileCoord.y));
		vec4 tileColor = texture(tiles, tilePos / vec2(SWAN_CHUNK_WIDTH, SWAN_CHUNK_HEIGHT));
		float tileID = floor((tileColor.r * 256.0 + tileColor.g) * 256.0);

		// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
		vec2 offset = v_tileCoord - tilePos;
		vec2 pixoffset = (1.0 - offset * 2.0) / (float(SWAN_TILE_SIZE) * 16.0);
		vec2 atlasPos = vec2(
			pixoffset.x + tileID + offset.x,
			pixoffset.y + floor(tileID / tileAtlasSize.x) + offset.y);

		fragColor = texture(tileAtlas, atlasPos / tileAtlasSize);
	}
)glsl";

const char *chunkShadowVx = R"glsl(
	#define CHUNK_WIDTH 64
	#define CHUNK_HEIGHT 64

	in vec2 vertex;
	uniform mat3 camera;
	uniform vec2 pos;
	out vec2 v_texCoord;

	void main() {
		vec3 pos = camera * vec3(pos + vertex, 1);
		gl_Position = vec4(pos.xy, 0, 1);
		v_texCoord = vertex / vec2(CHUNK_WIDTH, CHUNK_HEIGHT);
	}
)glsl";

const char *chunkShadowFr = R"glsl(
	in vec2 v_texCoord;
	uniform sampler2D tex;
	out vec4 fragColor;

	void main() {
		vec4 color = texture(tex, v_texCoord);
		fragColor = vec4(0, 0, 0, 1.0 - color.r);
	}
)glsl";

const char *tileVx = R"glsl(
	in vec2 vertex;
	uniform mat3 camera;
	uniform mat3 transform;
	out vec2 v_tileCoord;

	void main() {
		vec3 pos = camera * transform * vec3(vertex, 1);
		gl_Position = vec4(pos.xy, 0, 1);
		v_tileCoord = vertex;
	}
)glsl";

const char *tileFr = R"glsl(
	#define TILE_SIZE 32.0

	in vec2 v_tileCoord;
	uniform sampler2D tileAtlas;
	uniform vec2 tileAtlasSize;
	uniform float tileID;
	uniform float brightness;
	out vec4 fragColor;

	void main() {

		// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
		vec2 offset = v_tileCoord;
		vec2 pixoffset = (1.0 - offset * 2.0) / (TILE_SIZE * 16.0);
		vec2 atlasPos = vec2(
			pixoffset.x + tileID + offset.x,
			pixoffset.y + floor(tileID / tileAtlasSize.x) + offset.y);

		fragColor = vec4(texture(tileAtlas, atlasPos / tileAtlasSize).rgb * brightness, 1.0);
	}
)glsl";

const char *spriteVx = R"glsl(
	#define TILE_SIZE 32.0

	in vec2 vertex;
	uniform mat3 camera;
	uniform mat3 transform;
	uniform vec2 frameSize;
	uniform vec2 frameInfo; // frame count, frame index
	out vec2 v_texCoord;

	void main() {
		// Here, I'm basically treating 1/(TILE_SIZE*16) as half the size of a "pixel".
		// It's just an arbitrary small number, but it works as an offset to make sure
		// neighbouring parts of the atlas don't bleed into the frame we actually
		// want to draw due to (nearest neighbour) interpolation.
		float pixoffset = (1.0 - vertex.y * 2.0) / (frameSize.y * TILE_SIZE * 16.0);
		v_texCoord = vec2(
			vertex.x,
			(frameSize.y * frameInfo.y + (frameSize.y * vertex.y)) /
			(frameSize.y * frameInfo.x) + pixoffset);

		vec3 pos = camera * transform * vec3(vertex * frameSize, 1);
		gl_Position = vec4(pos.xy, 0, 1);
	}
)glsl";

const char *spriteFr = R"glsl(
	in vec2 v_texCoord;
	uniform sampler2D tex;
	out vec4 fragColor;

	void main() {
		fragColor = texture(tex, v_texCoord);
	}
)glsl";

const char *rectVx = R"glsl(
	in vec2 vertex;
	uniform mat3 camera;
	uniform vec2 pos;
	uniform vec2 size;
	out vec2 v_coord;

	void main() {
		vec3 pos = camera * vec3(pos + vertex * size, 1);
		gl_Position = vec4(pos.xy, 0, 1);
		v_coord = vertex * size;
	}
)glsl";

const char *rectFr = R"glsl(
	#define THICKNESS 0.02

	in vec2 v_coord;
	uniform vec2 size;
	uniform vec4 color;
	out vec4 fragColor;

	void main() {
		vec2 invCoord = size - v_coord;
		float minDist = min(v_coord.x, min(v_coord.y, min(invCoord.x, invCoord.y)));
		fragColor = color * float(minDist < THICKNESS);
	}
)glsl";

const char *blendVx = R"glsl(
	in vec2 vertex;
	in vec2 texCoord;
	out vec2 v_texCoord;

	void main() {
		gl_Position = vec4(vertex.xy, 0, 1);
		v_texCoord = texCoord;
	}
)glsl";

const char *blendFr = R"glsl(
	in vec2 v_texCoord;
	uniform sampler2D tex;
	out vec4 fragColor;

	void main() {
		//gl_FragColor = vec4(v_texCoord.x, v_texCoord.y, 0, 1);
		fragColor = texture(tex, v_texCoord);
	}
)glsl";

}
