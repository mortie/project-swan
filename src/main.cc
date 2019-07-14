#include <vector>
#include <time.h>

#include "common.h"
#include "Player.h"

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
	sf::RenderWindow window(sf::VideoMode(800, 600), "SFML works!");
	sf::Transform transform;
	transform.scale(UNIT_SIZE, UNIT_SIZE);

	Win win = { window, transform };

	Player player(Vec2(1, 1));

	double prevtime = getTime();
	double acc = 0;
	int fcount = 0;

	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed)
				window.close();
		}

		double now = getTime();
		double dt = now - prevtime;
		prevtime = now;
		acc += dt;
		fcount += 1;
		if (acc >= 1) {
			fprintf(stderr, "fps %i\n", fcount);
			acc = 0;
			fcount = 0;
		}

		window.clear(sf::Color(135, 206, 250));

		player.draw(win);
		player.update(dt);

		window.display();
	}

	return 0;
}
