#include "shaders.h"

namespace Cygnet::Shaders {

const char *texturedVx = R"glsl(
	uniform mat3 camera;
	attribute vec2 position;
	attribute vec2 texCoord;
	varying vec2 v_texCoord;
	void main() {
		vec3 pos = camera * vec3(position, 1);
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
