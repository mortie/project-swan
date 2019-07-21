#include <vector>
#include <time.h>
#include <unistd.h>

#include <swan/common.h>
#include <swan/Player.h>
#include <swan/World.h>
#include <swan/Game.h>

using namespace Swan;

static double getTime() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 600), "good gaem");
	window.setVerticalSyncEnabled(true);
	Win win(&window);

	Game game;
	game.loadMod("core.mod");

	game.createWorld();
	game.world_->setCurrentPlane(game.world_->addPlane());
	game.world_->player_ = new Player(Vec2(1, 1));

	Tile::ID tStone = game.world_->getTileID("core::stone");
	Tile::ID tGrass = game.world_->getTileID("core::grass");
	WorldPlane &plane = game.world_->getPlane(game.world_->current_plane_);
	for (int x = 1; x < 10; ++x) {
		for (int y = 3; y < 10; ++y) {
			if (y == 3)
				plane.setTileID(x, y, tGrass);
			else
				plane.setTileID(x, y, tStone);
		}
	}

	double prevtime = getTime();
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
		double now = getTime();
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
