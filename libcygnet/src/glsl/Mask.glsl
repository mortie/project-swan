uniform mat3 camera;
uniform vec2 pos;
uniform vec2 size;
uniform usampler2D tex;

// @Vertex
out vec2 v_texCoord;

void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0));
	vec2 vertex = lut[gl_VertexID];

	vec3 pos = camera * vec3(pos + vertex * size, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_texCoord = vertex / size;
}

// @Fragment
in vec2 v_texCoord;

void main() {
	uint color = texture(tex, v_texCoord).r;
	if (color >= 128u) {
		discard;
	}
}
