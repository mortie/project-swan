#include <cygnet/Window.h>
#include <cygnet/GlProgram.h>
#include <iostream>

const char *vertexSource = R"(
	attribute vec4 position;
	void main() {
		gl_Position = vec4(position.xyz, 1.0);
	}
)";

const char *fragmentSource = R"(
	precision mediump float;
	void main() {
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	}
)";

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	Cygnet::Window win("Hello Triangle", 640, 480);

	Cygnet::GlShader vertex(vertexSource, Cygnet::GlShader::Type::VERTEX);
	Cygnet::GlShader fragment(fragmentSource, Cygnet::GlShader::Type::FRAGMENT);
	Cygnet::GlProgram program({ vertex, fragment });
	GLuint positionAttrib = program.getLocation("position");
	program.use();

	GLfloat vertixes[] = {
		 0.0,  0.5,  0.0,
		-0.5, -0.5,  0.0,
		 0.5, -0.5, 0.0,
	};

	// Draw loop
	while (true) {
		SDL_Event evt;
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
				break;
			}
		}

		win.clear();

		glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, vertixes);
		glEnableVertexAttribArray(positionAttrib);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		win.flip();
	}

exit:
	SDL_Quit();
}
