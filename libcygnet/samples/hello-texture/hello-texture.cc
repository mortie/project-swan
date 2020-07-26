#include <cygnet/Window.h>
#include <cygnet/GlWrappers.h>
#include <cygnet/builtins.h>
#include <cygnet/glutil.h>
#include <iostream>

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	Cygnet::Deferred<SDL_Quit> sdl;
	Cygnet::Window win("Hello Texture", 640, 480);

	Cygnet::GlProgram program(Cygnet::builtinTextureVertex(), Cygnet::builtinTextureFragment());
	program.use();

	GLfloat vertexes[] = {
		-0.5f,  0.5f,  0.0f, // pos 0: top left
		 0.0f,  0.0f,        // tex 0: top left
		-0.5f, -0.5f,  0.0f, // pos 1: bottom left
		 0.0f,  1.0f,        // tex 1: bottom left
		 0.5f, -0.5f,  0.0f, // pos 2: bottom right
		 1.0f,  1.0f,        // tex 2: bottom right
		 0.5f,  0.5f,  0.0f, // pos 3: top right
		 1.0f,  0.0f,        // tex 3: top right
	};

	GLushort indexes[] = {
		0, 1, 2, // top left -> bottom left -> bottom right
		2, 3, 0, // bottom right -> top right -> top left
	};

	uint8_t image[] = {
		0xff, 0xff, 0xff,  0xaa, 0xaa, 0xaa,  0xff, 0x00, 0xba,  0, 0, 0,
		0xaa, 0xaa, 0xaa,  0xff, 0x00, 0x00,  0x00, 0x00, 0xff,  0, 0, 0,
		0xa0, 0x55, 0x77,  0x00, 0xf0, 0x0f,  0xff, 0x00, 0xff,  0, 0, 0,
	};

	Cygnet::GlTexture tex;
	tex.upload(3, 3, image, GL_RGB);

	GLint positionLoc = program.attribLoc("position", 0);
	GLint texCoordLoc = program.attribLoc("texCoord", 1);
	GLint texLoc = program.uniformLoc("tex");

	glUniform1i(texLoc, 0);

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

		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertexes);
		glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vertexes[3]);
		Cygnet::glCheck();

		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(texCoordLoc);
		Cygnet::glCheck();

		tex.bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indexes);
		Cygnet::glCheck();

		win.flip();
	}

exit:
	return EXIT_SUCCESS;
}
