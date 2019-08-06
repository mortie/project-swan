#include <vector>
#include <time.h>
#include <unistd.h>
#include <stdio.h>

#include <swan/common.h>
#include <swan/World.h>
#include <swan/Game.h>
#include <swan/Timer.h>

using namespace Swan;

int main() {
	sf::Image icon;
	if (!icon.loadFromFile("assets/icon.png")) {
		fprintf(stderr, "Failed to load image 'icon.png'\n");
		abort();
	}

	sf::RenderWindow window(sf::VideoMode(800, 600), "Project: SWAN");
	window.setVerticalSyncEnabled(true);
	window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
	Win win(&window);

	Game::initGlobal();

	Game game(win);
	game.loadMod("core.mod");

	game.createWorld("core::default");

	double prevtime = Timer::now();
	double fpsAcc = 0;
	double tickAcc = 0;
	int fcount = 0;
	int slowFrames = 0;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			} else if (event.type == sf::Event::Resized) {
				sf::FloatRect visibleArea(0, 0, event.size.width, event.size.height);
				window.setView(sf::View(visibleArea));
			}
		}

		// Display FPS
		double now = Timer::now();
		double dt = now - prevtime;
		prevtime = now;
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
				game.tick();
			}
		}

		window.clear();
		game.draw();
		window.display();
	}

	return 0;
}
