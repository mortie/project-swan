#define THICKNESS 0.02

uniform mat3 camera;
uniform vec2 pos;
uniform vec2 size;
uniform vec4 color;

// @Vertex
in vec2 vertex;
out vec2 v_coord;

void main() {
	vec3 pos = camera * vec3(pos + vertex * size, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_coord = vertex * size;
}

// @Fragment
in vec2 v_coord;
out vec4 fragColor;

void main() {
	vec2 invCoord = size - v_coord;
	float minDist = min(v_coord.x, min(v_coord.y, min(invCoord.x, invCoord.y)));
	fragColor = color * float(minDist < THICKNESS);
}
