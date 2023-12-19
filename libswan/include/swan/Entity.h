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

	virtual ~Entity() = default;

	Entity &operator=(Entity &&) = default;

	virtual void draw(const Context &ctx, Cygnet::Renderer &rnd) {}
	virtual void ui() {}
	virtual void update(const Context &ctx, float dt) {}
	virtual void tick(const Context &ctx, float dt) {}
	virtual void onDespawn(const Context &ctx) {}

	virtual void deserialize(const Swan::Context &ctx, const PackObject &obj) {}
	virtual PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) { return {}; }

	template<typename T>
	auto trait() {
		using Tag = typename T::Tag;
		T *t = dynamic_cast<T *>(this);
		if (!t) {
			return (decltype(&t->get(Tag{})))nullptr;
		}
		return &t->get(Tag{});
	}
};

}
