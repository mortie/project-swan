uniform mat3 camera;
uniform vec2 pos;
uniform sampler2D tileAtlas;
uniform uvec2 tileAtlasSize;
uniform usampler2D tiles;

// @Vertex
out vec2 v_texCoord;

void main() {
	vec2 lut[6] = vec2[](
		vec2(0, 0),
		vec2(0, 1),
		vec2(1, 1),
		vec2(1, 1),
		vec2(1, 0),
		vec2(0, 0));

	vec2 relPos = lut[gl_VertexID % 6];
	uint tileIndex = uint(gl_VertexID / 6);
	uvec2 tilePos = uvec2(
		tileIndex % uint(SWAN_CHUNK_WIDTH),
		tileIndex / uint(SWAN_CHUNK_WIDTH));

	vec2 vertex = vec2(tilePos) + relPos;
	vec3 p = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(p.xy, 0, 1);

	uint tileID = texture(tiles,
		(vec2(tilePos) + vec2(0.5, 0.5)) /
		vec2(SWAN_CHUNK_WIDTH, SWAN_CHUNK_HEIGHT)).r;
	vec2 atlasPos =
		vec2(tileID % tileAtlasSize.x, tileID / tileAtlasSize.x) +
		(relPos * 0.99 + vec2(0.005, 0.005));
	v_texCoord = atlasPos / tileAtlasSize;
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	fragColor = texture(tileAtlas, v_texCoord);
}
