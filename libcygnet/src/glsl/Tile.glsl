uniform mat3 camera;
uniform mat3 transform;
uniform sampler2D tileAtlas;
uniform vec2 tileAtlasSize;
uniform float tileID;
uniform float brightness;

// @Vertex
in vec2 vertex;
out vec2 v_atlasPos;

void main() {
	vec3 pos = camera * transform * vec3(vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);

	// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
	vec2 pixoffset = (1.0 - vertex * 2.0) / (float(SWAN_TILE_SIZE) * 16.0);
	v_atlasPos = vec2(
		pixoffset.x + tileID + vertex.x,
		pixoffset.y + floor(tileID / tileAtlasSize.x) + vertex.y);
}

// @Fragment
in vec2 v_atlasPos;
out vec4 fragColor;

void main() {
	fragColor = vec4(texture(tileAtlas, v_atlasPos / tileAtlasSize).rgb * brightness, 1.0);
}
