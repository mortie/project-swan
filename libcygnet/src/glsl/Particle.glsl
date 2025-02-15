uniform mat3 camera;
uniform vec2 pos;
uniform vec2 size;
uniform vec4 fill;

// @Vertex
void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0));
	vec2 vertex = lut[gl_VertexID];

	vec3 pos = camera * vec3(pos + vertex * size, 1);
	gl_Position = vec4(pos.xy, 0, 1);
}

// @Fragment
out vec4 fragColor;

void main() {
	fragColor = fill;
}
