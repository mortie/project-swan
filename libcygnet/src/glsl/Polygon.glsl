uniform mat3 camera;
uniform vec4 fill;

// @Vertex
in vec2 vertex;

void main() {
	vec3 pos = camera * vec3(vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
}

// @Fragment
out vec4 fragColor;

void main() {
	fragColor = fill;
}
