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

	Tile::TileID tStone = game.world_->getTileID("core::stone");
	WorldPlane &plane = game.world_->getPlane(game.world_->current_plane_);
	for (int x = 1; x < 10; ++x) {
		for (int y = 1; y < 10; ++y) {
			plane.setTile(x, y, tStone);
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
		if (fpsAcc >= 1) {
			fprintf(stderr, "FPS: %i\n", fcount);
			fpsAcc -= 1;
			fcount = 0;
		}

		game.update(dt);

		// Call tick TICK_RATE times per second
		tickAcc += dt;
		while (tickAcc >= 1.0 / TICK_RATE) {
			tickAcc -= 1.0 / TICK_RATE;
			game.tick();
		}

		window.clear(sf::Color(135, 206, 250));
		game.draw(win);
		window.display();
	}

	return 0;
}
