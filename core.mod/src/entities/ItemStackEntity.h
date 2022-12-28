#pragma once

#include <swan/swan.h>

class ItemStackEntity final: public Swan::PhysicsEntity {
public:
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item);
	ItemStackEntity(const Swan::Context &ctx, const PackObject &obj);

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

	Swan::Item *item() { return item_; }

private:
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.5, 0.5);
	static constexpr float DESPAWN_TIME = 5 * 60;
	static constexpr float BOUNCINESS = 0.6;

	ItemStackEntity(): PhysicsEntity(SIZE) {}

	float despawnTimer_ = DESPAWN_TIME;
	Swan::Item *item_ = NULL;
};
