uniform mat3 camera;
uniform vec2 pos;
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
	v_texCoord = vertex;

	vec3 pos = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	uvec4 col = texture(tex, v_texCoord);
	if (col.r > 0u) {
		fragColor = col;
	} else {
		discard;
	}
}
