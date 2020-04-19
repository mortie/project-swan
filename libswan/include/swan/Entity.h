#pragma once

#include <memory>
#include <optional>
#include <msgpack.hpp>

#include "common.h"
#include "log.h"
#include "traits/BodyTrait.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity: NonCopyable {
public:
	using PackObject = std::unordered_map<std::string_view, msgpack::object>;

	struct Factory {
		const std::string name;
		std::unique_ptr<Entity> (*create)(const Context &ctx, const PackObject &obj);
	};

	Entity() = default;
	Entity(Entity &&) = default;

	Entity &operator=(Entity &&) = default;

	void despawn(const Swan::Context &ctx);

	virtual ~Entity() = default;

	virtual void draw(const Context &ctx, Win &win) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick(const Context &ctx, float dt) {}
	virtual void onDespawn(const Context &ctx) {}

	virtual void deserialize(const Swan::Context &ctx, const PackObject &obj) {}
	virtual PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) { return {}; }

	size_t index_;
	size_t generation_;
};

class PhysicsEntity: public Entity, public BodyTrait::HasBody {
public:
	PhysicsEntity(Vec2 size, float mass):
		body_(size, mass) {}

	virtual BodyTrait::Body &getBody() override { return body_; }

	virtual void update(const Context &ctx, float dt) override {
		body_.standardForces();
		body_.update(ctx, dt);
	}

protected:
	BodyTrait::PhysicsBody body_;
};

}
