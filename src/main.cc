#include <vector>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <swan/common.h>
#include <swan/World.h>
#include <swan/Game.h>
#include <swan/Timer.h>
#include <swan/Win.h>

#include <SFML/System/Clock.hpp>
#include <SFML/Audio.hpp>

using namespace Swan;

int main() {
	sf::Image icon;
	if (!icon.loadFromFile("assets/icon.png")) {
		fprintf(stderr, "Failed to load image 'icon.png'\n");
		abort();
	}

	// Cretate window
	sf::RenderWindow window(sf::VideoMode(800, 600), "Project: SWAN");
	window.setVerticalSyncEnabled(true);
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	Win win(&window);

	// Create music
	sf::SoundBuffer musicbuf;
	sf::Sound music;
	if (musicbuf.loadFromFile("assets/music/happy-1.wav")) {
		music.setBuffer(musicbuf);
		music.setLoop(true);
		music.play();
	} else {
		fprintf(stderr, "Failed to load music! Am very sad.\n");
	}

	Game::initGlobal();

	Game game(win);
	game.loadMod("core.mod");

	game.createWorld("core::default");

	sf::Clock clock;
	float fpsAcc = 0;
	float tickAcc = 0;
	int fcount = 0;
	int slowFrames = 0;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;

			case sf::Event::Resized:
				window.setView(sf::View(sf::FloatRect(
					0, 0, event.size.width, event.size.height)));
				break;

			case sf::Event::KeyPressed:
				game.onKeyPressed(event.key.code);
				break;

			case sf::Event::KeyReleased:
				game.onKeyReleased(event.key.code);
				break;

			case sf::Event::MouseMoved:
				game.onMouseMove(event.mouseMove.x, event.mouseMove.y);
				break;

			case sf::Event::MouseButtonPressed:
				game.onMousePressed(event.mouseButton.button);
				break;

			case sf::Event::MouseButtonReleased:
				game.onMouseReleased(event.mouseButton.button);
				break;

			default: break;
			}
		}

		float dt = clock.restart().asSeconds();

		// Display FPS
		fpsAcc += dt;
		fcount += 1;
		if (fpsAcc >= 4) {
			fprintf(stderr, "FPS: %.3f\n", fcount / 4.0);
			fpsAcc -= 4;
			fcount = 0;
		}

		if (dt > 0.1) {
			if (slowFrames == 0)
				fprintf(stderr, "Warning: delta time is too high! (%.3fs).\n", dt);
			slowFrames += 1;
		} else {
			if (slowFrames > 0) {
				if (slowFrames > 1)
					fprintf(stderr, "%i consecutive slow frames.\n", slowFrames);
				slowFrames = 0;
			}

			game.update(dt);

			// Call tick TICK_RATE times per second
			tickAcc += dt;
			while (tickAcc >= 1.0 / TICK_RATE) {
				tickAcc -= 1.0 / TICK_RATE;
				game.tick(1.0 / TICK_RATE);
			}
		}

		window.clear();
		game.draw();
		window.display();
	}

	return 0;
}
