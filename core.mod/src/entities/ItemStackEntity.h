#pragma once

#include <swan/swan.h>

class ItemStackEntity: public Swan::PhysicsEntity {
public:
	class Factory: public Swan::Entity::Factory {
		Swan::Entity *create(const Swan::Context &ctx, const Swan::SRF &params) override {
			return new ItemStackEntity(ctx, params);
		}
	};

	ItemStackEntity(const Swan::Context &ctx, const Swan::SRF &params);

	void draw(const Swan::Context &ctx, Swan::Win &win) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void readSRF(const Swan::Context &ctx, const Swan::SRF &srf) override;
	Swan::SRF *writeSRF(const Swan::Context &ctx) override;

private:
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.5, 0.5);
	static constexpr float DESPAWN_TIME = 5 * 60;

	float despawn_timer_ = DESPAWN_TIME;
	Swan::Item *item_ = NULL;
};
