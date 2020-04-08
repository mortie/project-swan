#pragma once

#include <swan/swan.h>

class ItemStackEntity: public Swan::PhysicsEntity {
public:
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item);
	ItemStackEntity(const Swan::Context &ctx, const PackObject &obj);

	void draw(const Swan::Context &ctx, Swan::Win &win) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

private:
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.5, 0.5);
	static constexpr float DESPAWN_TIME = 5 * 60;

	ItemStackEntity(): PhysicsEntity(SIZE, MASS) {
		PhysicsEntity::body_.bounciness_ = 0.6;
	}

	float despawn_timer_ = DESPAWN_TIME;
	Swan::Item *item_ = NULL;
};