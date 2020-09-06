#include <cygnet/Window.h>
#include <cygnet/GlWrappers.h>
#include <cygnet/builtins.h>
#include <cygnet/glutil.h>
#include <cygnet/Image.h>
#include <cygnet/RenderQueue.h>
#include <stdio.h>
#include <memory>
#include <vector>
#include <iostream>

const char *vertexShader = R"(
	uniform mat3 transform;
	attribute vec2 position;
	attribute vec2 texCoord;
	varying vec2 v_texCoord;
	void main() {
		vec3 pos = transform * vec3(position, 0);
		gl_Position = vec4(pos.x, pos.y, 0, 1);
		v_texCoord = texCoord;
	}
)";

const char *fragmentShader = R"(
	precision mediump float;
	varying vec2 v_texCoord;
	uniform sampler2D tex;
	void main() {
		gl_FragColor = texture2D(tex, v_texCoord);
	}
)";

enum class Key {
	UP, DOWN, LEFT, RIGHT, NONE,
};

struct State {
	bool keys[(int)Key::NONE]{};
	Cygnet::Window &win;
	Cygnet::GlProgram &program;
	Cygnet::RenderQueue &q;
};

class Entity {
public:
	virtual ~Entity() = default;
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
		if (state.keys[(int)Key::UP])
			fy -= 1;
		if (state.keys[(int)Key::DOWN])
			fy += 1;

		fy += vy_ * -0.9;
		fx += vx_ * -0.9;

		vx_ += fx * dt;
		vy_ += fy * dt;
		x_ += vx_ * dt;
		y_ += vy_ * dt;
	}

	void draw(State &state) override {
		state.q.show(x_, y_, image_.texture());
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

	Cygnet::GlProgram program(
		Cygnet::GlShader(Cygnet::GlShader::Type::VERTEX, vertexShader),
		Cygnet::GlShader(Cygnet::GlShader::Type::FRAGMENT, fragmentShader));
	program.use();

	Cygnet::RenderQueue q(program, 1/100.0);

	State state{
		.keys{},
		.win = win,
		.program = program,
		.q = q,
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

			case SDL_WINDOWEVENT:
				if (evt.window.event == SDL_WINDOWEVENT_RESIZED) {
					win.onResize(evt.window.data1, evt.window.data2);
				}
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

		q.setScale(win.xScale(), win.yScale());
		q.draw();
		win.flip();
	}

exit: ;
}
