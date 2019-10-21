#pragma once

#include <swan/swan.h>

class EntPlayer: public Swan::Entity {
public:
	class Factory: public Swan::Entity::Factory {
	public:
		Swan::Entity *create(const Swan::Context &ctx, const Swan::SRF &params) override {
			return new EntPlayer(ctx, params);
		}
	};

	EntPlayer(const Swan::Context &ctx, const Swan::SRF &params);

	std::optional<Swan::BoundingBox> getBounds() override { return body_.getBounds(); }

	void draw(const Swan::Context &ctx, Swan::Win &win) override;
	void update(const Swan::Context &ctx, float dt) override;
	void readSRF(const Swan::Context &ctx, const Swan::SRF &srf) override;
	Swan::SRF *writeSRF(const Swan::Context &ctx) override;

private:
	static constexpr float FORCE = 3000;
	static constexpr float JUMP_FORCE = 10;
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 FRICTION = Swan::Vec2(400, 50);
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.6, 1.9);

	enum class State {
		IDLE,
		RUNNING_L,
		RUNNING_R,
		COUNT,
	};

	State state_ = State::IDLE;
	Swan::Animation anims_[(int)State::COUNT];

	Swan::Timer jump_timer_;
	Swan::TilePos mouse_tile_;

	Swan::Body body_;
};
