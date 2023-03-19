// @Vertex
#define TILE_SIZE 32.0

in vec2 vertex;
uniform mat3 camera;
uniform mat3 transform;
uniform vec2 frameSize;
uniform vec2 frameInfo; // frame count, frame index
out vec2 v_texCoord;

void main() {
	// Here, I'm basically treating 1/(TILE_SIZE*16) as half the size of a "pixel".
	// It's just an arbitrary small number, but it works as an offset to make sure
	// neighbouring parts of the atlas don't bleed into the frame we actually
	// want to draw due to (nearest neighbour) interpolation.
	float pixoffset = (1.0 - vertex.y * 2.0) / (frameSize.y * TILE_SIZE * 16.0);
	v_texCoord = vec2(
		vertex.x,
		(frameSize.y * frameInfo.y + (frameSize.y * vertex.y)) /
		(frameSize.y * frameInfo.x) + pixoffset);

	vec3 pos = camera * transform * vec3(vertex * frameSize, 1);
	gl_Position = vec4(pos.xy, 0, 1);
}

// @Fragment
in vec2 v_texCoord;
uniform sampler2D tex;
out vec4 fragColor;

void main() {
	fragColor = texture(tex, v_texCoord);
}
