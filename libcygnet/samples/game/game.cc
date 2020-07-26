#include <cygnet/Window.h>
#include <cygnet/GlWrappers.h>
#include <cygnet/builtins.h>
#include <cygnet/glutil.h>
#include <cygnet/Image.h>
#include <stdio.h>
#include <memory>
#include <vector>

enum class Key {
	UP, DOWN, LEFT, RIGHT, NONE,
};

struct State {
	bool keys[(int)Key::NONE]{};
	Cygnet::Window &win;
	Cygnet::GlProgram &program;
};

class Entity {
public:
	virtual void update(State &state, float dt) = 0;
	virtual void draw(State &state) = 0;
};

class PlayerEntity: public Entity {
public:
	PlayerEntity(float x, float y): x_(x), y_(y) {}

	void update(State &state, float dt) override {
		float fx{}, fy{};

		if (state.keys[(int)Key::LEFT])
			fx -= 1;
		if (state.keys[(int)Key::RIGHT])
			fx += 1;

		vx_ += fx * dt;
		vy_ += fy * dt;
		x_ += vx_ * dt;
		y_ += vy_ * dt;
	}

	void draw(State &state) override {
		printf("\ram at (%f,%f)", x_, y_);
		fflush(stdout);

		const GLfloat vertexes[] = {
			x_ - 0.5f, y_ + 0.5f, 0.0f, // pos 0: top left
			0.0f,      0.0f,            // tex 0: top left
			x_ - 0.5f, y_ - 0.5f, 0.0f, // pos 1: bottom left
			0.0f,      1.0f,            // tex 1: bottom left
			x_ + 0.5f, y_ - 0.5f, 0.0f, // pos 2: bottom right
			1.0f,      1.0f,            // tex 2: bottom right
			x_ + 0.5f, y_ + 0.5f, 0.0f, // pos 3: top right
			1.0f,      0.0f,            // tex 3: top right
		};

		static const GLushort indexes[] = {
			0, 1, 2, // top left -> bottom left -> bottom right
			2, 3, 0, // bottom right -> top right -> top left
		};

		GLint positionLoc = state.program.attribLoc("position", 0);
		GLint texCoordLoc = state.program.attribLoc("texCoord", 1);
		GLint texLoc = state.program.uniformLoc("tex");

		glUniform1i(texLoc, 0);

		glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertexes);
		glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), &vertexes[3]);
		Cygnet::glCheck();

		glEnableVertexAttribArray(positionLoc);
		glEnableVertexAttribArray(texCoordLoc);
		Cygnet::glCheck();

		image_.texture().bind();
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indexes);
		Cygnet::glCheck();
	}

private:
	float x_, y_;
	float vx_{}, vy_{};

	Cygnet::Image image_{"libcygnet/samples/game/assets/player.png"};
};

Key keyFromSym(SDL_Keysym sym) {
	switch (sym.scancode) {
	case SDL_SCANCODE_W:
		return Key::UP;
	case SDL_SCANCODE_A:
		return Key::LEFT;
	case SDL_SCANCODE_S:
		return Key::DOWN;
	case SDL_SCANCODE_D:
		return Key::RIGHT;
	default:
		return Key::NONE;
	}
}

int main() {
	SDL_Init(SDL_INIT_VIDEO);
	Cygnet::Deferred<SDL_Quit> sdl;
	Cygnet::Window win("Game", 640, 480);

	Cygnet::GlProgram program(Cygnet::builtinTextureVertex(), Cygnet::builtinTextureFragment());
	program.use();

	State state{
		.keys{},
		.win = win,
		.program = program,
	};

	std::vector<std::unique_ptr<Entity>> entities;
	entities.emplace_back(std::make_unique<PlayerEntity>(0, 0));

	SDL_Event evt;
	while (true) {
		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				goto exit;
				break;

			case SDL_KEYDOWN:
				{
					Key key = keyFromSym(evt.key.keysym);
					if (key != Key::NONE) {
						state.keys[(int)key] = true;
					}
				}
				break;

			case SDL_KEYUP:
				{
					Key key = keyFromSym(evt.key.keysym);
					if (key != Key::NONE) {
						state.keys[(int)key] = false;
					}
				}
				break;
			}
		}

		win.clear();

		for (auto &ent: entities) {
			ent->update(state, 1/60.0);
		}

		for (auto &ent: entities) {
			ent->draw(state);
		}

		win.flip();
	}

exit: ;
}
