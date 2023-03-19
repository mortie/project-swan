uniform mat3 camera;
uniform vec2 pos;
uniform sampler2D tex;

// @Vertex
in vec2 vertex;
out vec2 v_texCoord;

void main() {
	vec3 pos = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_texCoord = vertex / vec2(SWAN_CHUNK_WIDTH, SWAN_CHUNK_HEIGHT);
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	vec4 color = texture(tex, v_texCoord);
	fragColor = vec4(0, 0, 0, 1.0 - color.r);
}
