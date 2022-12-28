#pragma once

#include <memory>
#include <optional>
#include <msgpack.hpp>

#include "common.h"
#include "log.h"
#include "traits/BodyTrait.h"
#include "traits/PhysicsTrait.h"

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

	virtual ~Entity() = default;

	Entity &operator=(Entity &&) = default;

	void despawn(const Swan::Context &ctx);

	virtual void draw(const Context &ctx, Cygnet::Renderer &rnd) {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick(const Context &ctx, float dt) {}
	virtual void onDespawn(const Context &ctx) {}

	virtual void deserialize(const Swan::Context &ctx, const PackObject &obj) {}
	virtual PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) { return {}; }

	size_t index_;
	size_t generation_;
};

class PhysicsEntity: public Entity, public BodyTrait, public PhysicsTrait {
public:
	PhysicsEntity(Vec2 size): body_({.size = size}) {}

	BodyTrait::Body &get(BodyTrait::Tag) override { return body_; }
	PhysicsTrait::Physics &get(PhysicsTrait::Tag) override { return physics_; }

	void physics(
			const Context &ctx, float dt,
			const BasicPhysics::Props &props) {

		physics_.standardForces(props.mass);
		physics_.update(ctx, dt, body_, props);
	}

protected:
	BodyTrait::Body body_;
	BasicPhysics physics_;
};

}
