uniform sampler2D tex;

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
	//gl_FragColor = vec4(v_texCoord.x, v_texCoord.y, 0, 1);
	fragColor = texture(tex, v_texCoord);
}
