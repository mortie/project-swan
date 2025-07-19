#pragma once

#include <swan/util.h>
#include "common.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Entity: NonCopyable {
public:
	Entity() = default;
	Entity(Entity &&) = default;

	virtual ~Entity() = default;

	Entity &operator=(Entity &&) = default;

	virtual void draw(Ctx &ctx, Cygnet::Renderer &rnd)
	{}
	virtual void update(Ctx &ctx, float dt)
	{}
	virtual void tick(Ctx &ctx, float dt)
	{}
	virtual void tick2(Ctx &ctx, float dt)
	{}
	virtual void onSpawn(Ctx &ctx)
	{}
	virtual void onDespawn(Ctx &ctx)
	{}
	virtual void onWorldLoaded(Ctx &ctx)
	{}
	virtual void drawDebug(Ctx &ctx)
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
