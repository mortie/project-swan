#pragma once

#include <swan/swan.h>

class EntPlayer: public Swan::Entity {
public:
	class Factory: public Swan::Entity::Factory {
	public:
		Swan::Entity *create(Swan::World &world, const Swan::Vec2 &pos) override {
			return new EntPlayer(world, pos);
		}
	};

	EntPlayer(Swan::World &world, const Swan::Vec2 &pos);

	const Swan::Vec2 &getPos() override { return body_.pos_; }

	void draw(Swan::Win &win) override;
	void update(Swan::WorldPlane &plane, float dt) override;

private:
	static constexpr float FORCE = 3000;
	static constexpr float JUMP_FORCE = 9;
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 FRICTION = Swan::Vec2(400, 0);
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.6, 1.9);

	enum class State {
		IDLE,
		RUNNING_L,
		RUNNING_R,
		COUNT,
	};

	State state_ = State::IDLE;
	Swan::Animation anims_[(int)State::COUNT];

	Swan::Body body_;
};
