uniform mat3 camera;
uniform mat3 transform;
uniform sampler2D tileAtlas;
uniform vec2 tileAtlasSize;
uniform float tileID;
uniform float brightness;

// @Vertex
in vec2 vertex;
out vec2 v_tileCoord;

void main() {
	vec3 pos = camera * transform * vec3(vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_tileCoord = vertex;
}

// @Fragment
in vec2 v_tileCoord;
out vec4 fragColor;

void main() {
	// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
	vec2 offset = v_tileCoord;
	vec2 pixoffset = (1.0 - offset * 2.0) / (float(SWAN_TILE_SIZE) * 16.0);
	vec2 atlasPos = vec2(
		pixoffset.x + tileID + offset.x,
		pixoffset.y + floor(tileID / tileAtlasSize.x) + offset.y);

	fragColor = vec4(texture(tileAtlas, atlasPos / tileAtlasSize).rgb * brightness, 1.0);
}
