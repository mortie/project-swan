#include "shaders.h"

namespace Cygnet::Shaders {

const char *texturedVx = R"glsl(
	uniform mat3 camera;
	uniform mat3 transform;
	attribute vec2 vertex;
	attribute vec2 texCoord;
	varying vec2 v_texCoord;
	void main() {
		vec3 pos = camera * transform * vec3(vertex, 1);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
		v_texCoord = texCoord;
	}
)glsl";

const char *texturedFr = R"glsl(
	precision mediump float;
	varying vec2 v_texCoord;
	uniform sampler2D tex;
	void main() {
		gl_FragColor = texture2D(tex, v_texCoord);
	}
)glsl";

}
