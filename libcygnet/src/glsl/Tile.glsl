uniform mat3 camera;
uniform mat3 transform;
uniform sampler2D tileAtlas;
uniform vec2 tileAtlasSize;
uniform usampler2D tileMap;
uniform float tileMapSize;
uniform uint tileID;
uniform float brightness;

// @Vertex
out vec2 v_atlasPos;

void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0));
	vec2 vertex = lut[gl_VertexID];

	vec3 pos = camera * transform * vec3(vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);

	vec2 assetPos = vec2((float(tileID) + 0.5f) / float(tileMapSize), 0.5f);
	uint assetID = texture(tileMap, assetPos).r;

	// 1/(TILE_SIZE*16) plays the same role here as in the sprite vertex shader.
	vec2 pixoffset = (1.0 - vertex * 2.0) / (float(SWAN_TILE_SIZE) * 16.0);
	v_atlasPos = vec2(
		pixoffset.x + assetID + vertex.x,
		pixoffset.y + floor(assetID / tileAtlasSize.x) + vertex.y);
}

// @Fragment
in vec2 v_atlasPos;
out vec4 fragColor;

void main() {
	vec4 pix = texture(tileAtlas, v_atlasPos / tileAtlasSize);
	fragColor = vec4(pix.rgb * brightness, 1.0) * pix.a;
}
