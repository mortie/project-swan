#pragma once

#include <memory>
#include <msgstream/msgstream.h>

#include "common.h"
#include "util.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity: NonCopyable {
public:
	struct Factory {
		const std::string name;
		std::unique_ptr<Entity> (*create)(
			const Context &ctx, MsgStream::MapParser &parser);
	};

	Entity() = default;
	Entity(Entity &&) = default;

	virtual ~Entity() = default;

	Entity &operator=(Entity &&) = default;

	virtual void draw(const Context &ctx, Cygnet::Renderer &rnd)
	{}
	virtual void ui(const Context &ctx)
	{}
	virtual void update(const Context &ctx, float dt)
	{}
	virtual void tick(const Context &ctx, float dt)
	{}
	virtual void onDespawn(const Context &ctx)
	{}

	virtual void serialize(const Swan::Context &ctx, MsgStream::MapBuilder &w)
	{}
	virtual void deserialize(const Swan::Context &ctx, MsgStream::MapParser &r)
	{}

	template<typename T>
	auto trait()
	{
		using Tag = typename T::Tag;
		T *t = dynamic_cast<T *>(this);
		if (!t) {
			return (decltype(&t->get(Tag{}))) nullptr;
		}
		return &t->get(Tag{});
	}
};

}
