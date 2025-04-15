uniform mat3 camera;
uniform vec2 pos;
uniform float gamma;
uniform sampler2D tex;

// @Vertex
out vec2 v_texCoord;

void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, float(SWAN_CHUNK_HEIGHT)),
		vec2(float(SWAN_CHUNK_WIDTH), 0.0),
		vec2(float(SWAN_CHUNK_WIDTH), float(SWAN_CHUNK_HEIGHT)));
	vec2 vertex = lut[gl_VertexID];

	vec3 pos = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_texCoord = vertex / vec2(SWAN_CHUNK_WIDTH, SWAN_CHUNK_HEIGHT);
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	vec4 color = texture(tex, v_texCoord);
	fragColor = vec4(0, 0, 0, 1.0f - (color.r * gamma));
}
