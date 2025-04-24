uniform mat3 camera;
uniform mat3 transform;
uniform vec2 translate;
uniform vec2 frameSize;
uniform vec2 frameInfo; // frame count, frame index
uniform sampler2D tex;

// @Vertex
out vec2 v_texCoord;

void main() {
	vec2 lut[4] = vec2[](
		vec2(0.0, 0.0),
		vec2(0.0, 1.0),
		vec2(1.0, 0.0),
		vec2(1.0, 1.0));
	vec2 vertex = lut[gl_VertexID];

	// Here, I'm basically treating 1/(TILE_SIZE*16) as half the size of a "pixel".
	// It's just an arbitrary small number, but it works as an offset to make sure
	// neighbouring parts of the atlas don't bleed into the frame we actually
	// want to draw due to (nearest neighbour) interpolation.
	float pixoffset = (1.0 - vertex.y * 2.0) / (frameSize.y * float(SWAN_TILE_SIZE) * 16.0);
	v_texCoord = vec2(
		vertex.x,
		(frameSize.y * frameInfo.y + (frameSize.y * vertex.y)) /
		(frameSize.y * frameInfo.x) + pixoffset);

	vec3 pos = transform * vec3(vertex * frameSize, 1);
	pos.x += translate.x;
	pos.y += translate.y;
	pos = camera * pos;
	gl_Position = vec4(pos.xy, 0, 1);
}

// @Fragment
in vec2 v_texCoord;
out vec4 fragColor;

void main() {
	fragColor = texture(tex, v_texCoord);
}
