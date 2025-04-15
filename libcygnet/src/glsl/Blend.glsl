uniform sampler2D tex;
uniform float desaturate;

// @Vertex
in vec2 vertex;
in vec2 texCoord;
out vec2 v_texCoord;

void main() {
	vec4 lut[4] = vec4[](
		vec4(-1.0, -1.0, 0.0, 0.0),
		vec4(-1.0,  1.0, 0.0, 1.0),
		vec4( 1.0, -1.0, 1.0, 0.0),
		vec4( 1.0,  1.0, 1.0, 1.0));
	vec4 vertex = lut[gl_VertexID];
	gl_Position = vec4(vertex.xy, 0, 1);
	v_texCoord = vertex.zw;
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	vec4 color = texture(tex, v_texCoord);
	vec3 rgb = color.rgb;
	float avg = (rgb.r + rgb.g + rgb.b) / 3.0f;
	vec3 grey = vec3(avg, avg, avg);
	rgb = (rgb * (1.0 - desaturate)) + (grey * desaturate);
	fragColor = vec4(rgb.r, rgb.g, rgb.b, color.a);
}
