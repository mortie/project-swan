#include "Tile.h"

namespace Swan{

Tile::Builder &Tile::Builder::withName(std::string name)
{
	this->name = std::move(name);
	return *this;
}

Tile::Builder &Tile::Builder::withImage(std::string image)
{
	this->image = std::move(image);
	return *this;
}

Tile::Builder &Tile::Builder::withFluidMask(std::string fluidMask)
{
	this->fluidMask = std::move(fluidMask);
	return *this;
}

Tile::Builder &Tile::Builder::withIsSolid(bool isSolid)
{
	this->isSolid = std::move(isSolid);
	return *this;
}

Tile::Builder &Tile::Builder::withIsOpaque(bool isOpaque)
{
	this->isOpaque = std::move(isOpaque);
	return *this;
}

Tile::Builder &Tile::Builder::withIsSupportV(bool isSupportV)
{
	this->isSupportV = std::move(isSupportV);
	return *this;
}

Tile::Builder &Tile::Builder::withIsSupportH(bool isSupportH)
{
	this->isSupportH = std::move(isSupportH);
	return *this;
}

Tile::Builder &Tile::Builder::withIsPlatform(bool isPlatform)
{
	this->isPlatform = std::move(isPlatform);
	return *this;
}

Tile::Builder &Tile::Builder::withIsReplacable(bool isReplacable)
{
	this->isReplacable = std::move(isReplacable);
	return *this;
}

Tile::Builder &Tile::Builder::withLightLevel(float lightLevel)
{
	this->lightLevel = std::move(lightLevel);
	return *this;
}

Tile::Builder &Tile::Builder::withTemperature(float temperature)
{
	this->temperature = std::move(temperature);
	return *this;
}

Tile::Builder &Tile::Builder::withBreakableBy(ToolSet breakableBy)
{
	this->breakableBy = std::move(breakableBy);
	return *this;
}

Tile::Builder &Tile::Builder::withStepSound(std::string stepSound)
{
	this->stepSound = std::move(stepSound);
	return *this;
}

Tile::Builder &Tile::Builder::withPlaceSound(std::string placeSound)
{
	this->placeSound = std::move(placeSound);
	return *this;
}

Tile::Builder &Tile::Builder::withBreakSound(std::string breakSound)
{
	this->breakSound = std::move(breakSound);
	return *this;
}

Tile::Builder &Tile::Builder::withDroppedItem(std::string droppedItem)
{
	this->droppedItem = std::move(droppedItem);
	return *this;
}

Tile::Builder &Tile::Builder::withTileEntity(std::string tileEntity)
{
	this->tileEntity = std::move(tileEntity);
	return *this;
}

Tile::Builder &Tile::Builder::withOnSpawn(bool (*onSpawn)(Ctx &, TilePos))
{
	this->onSpawn = onSpawn;
	return *this;
}

Tile::Builder &Tile::Builder::withOnBreak(void (*onBreak)(Ctx &, TilePos))
{
	this->onBreak = onBreak;
	return *this;
}

Tile::Builder &Tile::Builder::withOnTileUpdate(
	void (*onTileUpdate)(Ctx &, TilePos))
{
	this->onTileUpdate = onTileUpdate;
	return *this;
}

Tile::Builder &Tile::Builder::withOnActivate(
	void (*onActivate)(Ctx &, TilePos, ActivateMeta))
{
	this->onActivate = onActivate;
	return *this;
}

Tile::Builder &Tile::Builder::withOnWorldTick(
	void (*onWorldTick)(Ctx &, TilePos))
{
	this->onWorldTick = onWorldTick;
	return *this;
}

Tile::Builder &Tile::Builder::withFluidCollision(
	std::shared_ptr<FluidCollision> fluidCollision)
{
	this->fluidCollision = std::move(fluidCollision);
	return *this;
}

Tile::Builder &Tile::Builder::withTraits(std::shared_ptr<Traits> traits)
{
	this->traits = std::move(traits);
	return *this;
}

}
