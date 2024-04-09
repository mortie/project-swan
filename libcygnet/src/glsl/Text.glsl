uniform mat3 camera;
uniform mat3 transform;
uniform sampler2D textAtlas;
uniform uvec2 textAtlasSize;
uniform ivec2 positionOffset;
uniform uvec2 charPosition;
uniform uvec2 charSize;
uniform vec4 color;
uniform float textScale;

// @Vertex
out vec2 v_atlasPos;

void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0));
	vec2 vertex = lut[gl_VertexID];

	vec3 pos = camera * transform *
		vec3((positionOffset + charSize * vertex) * textScale, 1);
	gl_Position = vec4(pos.xy, 0, 1);

	vertex += vec2(0.01, 0.01);
	vertex *= 0.98;
	v_atlasPos = vec2(charPosition + charSize * vertex) / textAtlasSize;
}

// @Fragment
in vec2 v_atlasPos;
out vec4 fragColor;

void main() {
	vec4 pix = texture(textAtlas, v_atlasPos);
	fragColor = vec4(color.rgb, color.a * pix.r);
}
