#include <vector>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "Player.h"
#include "Game.h"

double getTime() {
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

template<typename T>
void draw_ents(std::vector<T> ents) {
	for (auto &ent: ents)
		ent.draw();
}

int main() {
	sf::RenderWindow window(sf::VideoMode(800, 600), "good gaem");
	window.setVerticalSyncEnabled(true);
	sf::Transform transform;
	transform.scale(UNIT_SIZE, UNIT_SIZE);

	Win win = { window, transform };

	Game game;
	game.player_ = new Player(Vec2(1, 1));
	game.current_plane_ = new WorldPlane();
	game.planes_.push_back(game.current_plane_);

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
