#include <vector>
#include <time.h>
#include <unistd.h>

#include <swan/common.h>
#include <swan/World.h>
#include <swan/Game.h>
#include <swan/Timer.h>

using namespace Swan;

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 600), "good gaem");
	window.setVerticalSyncEnabled(true);
	Win win(&window);

	Game::initGlobal();

	Game game;
	game.loadMod("core.mod");

	game.createWorld("core::default");

	double prevtime = Timer::now();
	double fpsAcc = 0;
	double tickAcc = 0;
	int fcount = 0;

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
			fprintf(stderr, "Warning: delta time is too high! (%.3fs).\n", dt);
		} else {
			game.update(dt);

			// Call tick TICK_RATE times per second
			tickAcc += dt;
			while (tickAcc >= 1.0 / TICK_RATE) {
				tickAcc -= 1.0 / TICK_RATE;
				game.tick();
			}
		}

		window.clear();
		game.draw(win);
		window.display();
	}

	return 0;
}
