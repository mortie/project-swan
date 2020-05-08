#include <cygnet/Window.h>
#include <cygnet/GlWrappers.h>
#include <cygnet/BuiltinShaders.h>
#include <cygnet/glutil.h>
#include <iostream>

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	Cygnet::Deferred<SDL_Quit> sdl;
	Cygnet::Window win("Hello Triangle", 640, 480);

	Cygnet::BuiltinShaders shaders;
	Cygnet::GlProgram program(shaders.textureVertex, shaders.textureFragment);
	program.use();

	GLfloat vertexes[] = {
		-0.5f, -0.5f,  0.0f, // pos 0: top left
		 0.0f,  0.0f,        // tex 0: top left
		-0.5f,  0.5f,  0.0f, // pos 1: bottom left
		 0.0f,  1.0f,        // tex 1: bottom left
		 0.5f,  0.5f,  0.0f, // pos 2: bottom right
		 1.0f,  1.0f,        // tex 2: bottom right
		 0.5f, -0.5f,  0.0f, // pos 3: top right
		 1.0f,  0.0f,        // tex 3: top right
	};

	GLushort indexes[] = {
		0, 1, 2, // top left -> bottom left -> bottom right
		2, 3, 0, // bottom right -> top right -> top left
	};

	uint8_t image[] = {
		0xff, 0xff, 0xff,  0xaa, 0xaa, 0xaa,  0xff, 0x00, 0xba,
		0xaa, 0xaa, 0xaa,  0xff, 0x00, 0x00,  0x00, 0x00, 0xff,
		0xa0, 0x55, 0x77,  0x00, 0xf0, 0x0f,  0xff, 0x00, 0xff,
	};

	GLuint texId;
	glGenTextures(1, &texId);
	glBindTexture(GL_TEXTURE_2D, texId);
	Cygnet::glCheck();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 3, 3, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	Cygnet::glCheck();

	GLint positionLoc = program.attribLocation("position");
	GLint texCoordLoc = program.attribLocation("texCoord");
	GLint texLoc = program.uniformLocation("tex");

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

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texId);
		glUniform1i(texLoc, 0);
		Cygnet::glCheck();

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indexes);
		Cygnet::glCheck();

		win.flip();
	}

exit:
	return EXIT_SUCCESS;
}
