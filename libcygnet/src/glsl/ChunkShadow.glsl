// @Vertex
#define CHUNK_WIDTH 64
#define CHUNK_HEIGHT 64

in vec2 vertex;
uniform mat3 camera;
uniform vec2 pos;
out vec2 v_texCoord;

void main() {
	vec3 pos = camera * vec3(pos + vertex, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_texCoord = vertex / vec2(CHUNK_WIDTH, CHUNK_HEIGHT);
}

// @Fragment
in vec2 v_texCoord;
uniform sampler2D tex;
out vec4 fragColor;

void main() {
	vec4 color = texture(tex, v_texCoord);
	fragColor = vec4(0, 0, 0, 1.0 - color.r);
}
