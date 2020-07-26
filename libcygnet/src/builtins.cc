#include "builtins.h"

namespace Cygnet {

const GlShader &builtinTextureVertex() {
	static GlShader shader(GlShader::Type::VERTEX, R"(
		attribute vec4 position;
		attribute vec2 texCoord;
		varying vec2 v_texCoord;
		void main() {
			gl_Position = position;
			v_texCoord = texCoord;
		}
	)");
	return shader;
}

const GlShader &builtinTextureFragment() {
	static GlShader shader(GlShader::Type::FRAGMENT, R"(
		precision mediump float;
		varying vec2 v_texCoord;
		uniform sampler2D tex;
		void main() {
			gl_FragColor = texture2D(tex, v_texCoord);
		}
	)");
	return shader;
}

const GlShader &builtinWhiteFragment() {
	static GlShader shader(GlShader::Type::FRAGMENT, R"(
		precision mediump float;
		void main() {
			gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		}
	)");
	return shader;
}

}
