#include "Body.h"

void Body::outline(Win &win) {
	win.setPos(pos_);

	sf::RectangleShape rect(size_);
	rect.setFillColor(sf::Color::Transparent);
	rect.setOutlineColor(sf::Color(128, 128, 128));
	rect.setOutlineThickness(2 / UNIT_SIZE);
	win.draw(rect);
}

void Body::update(float dt) {
	vel_ += force_ * dt;
	pos_ += vel_ * dt;
	force_ = { 0, 0 };
}
