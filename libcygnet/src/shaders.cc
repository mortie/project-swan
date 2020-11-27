#include "shaders.h"

namespace Cygnet::Shaders {

const char *basicVx = R"glsl(
	uniform mat3 transform;
	attribute vec2 position;
	void main() {
		vec3 pos = transform * vec3(position, 0);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
	}
)glsl";

const char *texturedVx = R"glsl(
	uniform mat3 transform;
	attribute vec2 position;
	attribute vec2 texCoord;
	varying vec2 v_texCoord;
	void main() {
		vec3 pos = transform * vec3(position, 0);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
		v_texCoord = texCoord;
	}
)glsl";

const char *solidColorFr = R"glsl(
	precision mediump float;
	uniform vec4 color;
	void main() {
		gl_FragColor = color;
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
