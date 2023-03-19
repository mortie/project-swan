uniform mat3 camera;
uniform vec2 pos;
uniform sampler2D tileAtlas;
uniform vec2 tileAtlasSize;
uniform sampler2D tiles;

// @Vertex
in vec2 vertex;
out vec2 v_tileCoord;

void main() {
	vec3 p = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(p.xy, 0, 1);
	v_tileCoord = vertex;
}

// @Fragment
in vec2 v_tileCoord;
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
