#pragma once

#include <swan/swan.h>

class EntPlayer: public Swan::Entity {
public:
	class Factory: public Swan::Entity::Factory {
	public:
		Swan::Entity *create(const Swan::Vec2 &pos) override { return new EntPlayer(pos); }
	};

	Swan::Body body_;

	EntPlayer(Swan::Vec2 pos):
		body_(pos, SIZE, MASS) {}

	void draw(Swan::Win &win) override;
	void update(Swan::WorldPlane &plane, float dt) override;

private:
	static constexpr float FORCE = 600;
	static constexpr float FRICTION = 100;
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(1, 2);
};
