// @Vertex
in vec2 vertex;
uniform mat3 camera;
uniform vec2 pos;
uniform vec2 size;
out vec2 v_coord;

void main() {
	vec3 pos = camera * vec3(pos + vertex * size, 1);
	gl_Position = vec4(pos.xy, 0, 1);
	v_coord = vertex * size;
}

// @Fragment
#define THICKNESS 0.02

in vec2 v_coord;
uniform vec2 size;
uniform vec4 color;
out vec4 fragColor;

void main() {
	vec2 invCoord = size - v_coord;
	float minDist = min(v_coord.x, min(v_coord.y, min(invCoord.x, invCoord.y)));
	fragColor = color * float(minDist < THICKNESS);
}
