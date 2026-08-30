#pragma once

#include <swan/util.h>
#include <capnp/message.h>
#include <capnp/serialize.h>
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

	virtual void serialize(Ctx &ctx, capnp::MessageBuilder &mb)
	{}
	virtual void deserialize(Ctx &ctx, capnp::MessageReader &mr)
	{}

	virtual bool hasUpdated()
	{ return true; }
	virtual void serializeUpdates(Ctx &ctx, capnp::MessageBuilder &mb)
	{ serialize(ctx, mb); }
	virtual void deserializeUpdates(Ctx &ctx, capnp::MessageReader &mr)
	{ deserialize(ctx, mr); }

	template<typename T>
	using TraitType = decltype(std::declval<T>().get(typename T::Tag{}));

	// Most traits let you retrieve a reference to something within the entity.
	// For these, return a nullable pointer.
	template<typename T>
	std::remove_reference_t<TraitType<T>> *trait()
		requires(std::is_reference_v<TraitType<T>>)
	{
		using Tag = typename T::Tag;
		T *t = dynamic_cast<T *>(this);
		if (t) {
			return &t->get(Tag{});
		} else {
			return (std::remove_reference_t<TraitType<T>> *)nullptr;
		}
	}

	// Some traits let you retrieve an actual object.
	// For these, return an optional.
	template<typename T>
	std::optional<TraitType<T>> trait()
		requires(!std::is_reference_v<TraitType<T>>)
	{
		using Tag = typename T::Tag;
		T *t = dynamic_cast<T *>(this);
		if (t) {
			return std::optional(t->get(Tag{}));
		} else {
			return std::optional<TraitType<T>>(std::nullopt);
		}
	}
};

}
