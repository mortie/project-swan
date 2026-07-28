#include "IncandescentLampTileEntity.h"
#include "cygnet/util.h"
#include "traits/PowerBufferTrait.h"
#include <cstdlib>
#include <limits>
#include <numbers>

namespace CoreMod {

// Properties of tungsten
static constexpr float FILAMENT_SPECIFIC_HEAT_CAPACITY = 132.017; // joules per (kg * kelvin)
static constexpr float FILAMENT_DENSITY = 19254; // kg per m^3
static constexpr float FILAMENT_EMISSIVITY = 0.45; // Roughly

// Tweakable values for our specific filament
static constexpr float FILAMENT_LENGTH = 25e-3; // m
static constexpr float FILAMENT_DIAMETER = 50e-6; // m

// Current limit,
// helps avoid extreme behavior during inrush
static constexpr Ampere FILAMENT_CURRENT_LIMIT = 1;

// Useful computed properties
static constexpr float FILAMENT_RADIUS = FILAMENT_DIAMETER / 2; // m
static constexpr float FILAMENT_CROSS_SECTION = ( // m^2
	std::numbers::pi * FILAMENT_RADIUS * FILAMENT_RADIUS);
static constexpr float FILAMENT_CIRCUMFERENCE = std::numbers::pi * FILAMENT_DIAMETER; // m
static constexpr float FILAMENT_AREA = FILAMENT_CIRCUMFERENCE * FILAMENT_LENGTH; // m^2
static constexpr float FILAMENT_WEIGHT = ( // kg
	FILAMENT_CROSS_SECTION * FILAMENT_LENGTH * FILAMENT_DENSITY);
static constexpr float FILAMENT_HEAT_CAPACITY = ( // joules per kelvin
	FILAMENT_SPECIFIC_HEAT_CAPACITY * FILAMENT_WEIGHT);

static constexpr float filamentResistance(float kelvin)
{
	// https://hypertextbook.com/facts/2004/DeannaStewart.shtml
	// Lide, D. CRC Handbook of Chemistry and Physics, 75 edition. CRC Press, 1995: 12.
	// R_W = 48.0 (1 + 4.8297 × 10^−3 T + 1.663 × 10^−6 T^2
	float nanoResistivity = 48.0 * (1 + (4.8297e-3 * kelvin) + (1.663e-6 * kelvin * kelvin));
	float resistivity = nanoResistivity / 1e9;

	// resistance = (resistivity * length) / x-section
	return (resistivity * FILAMENT_LENGTH) / FILAMENT_CROSS_SECTION;
}

void IncandescentLampTileEntity::onSpawn(Swan::Ctx &ctx)
{
	powerSource_ = ctx.plane.entities().getTileEntity(tileEntity_.pos.add(1, 0));
	if (!powerSource_) {
		powerSource_ = ctx.plane.entities().getTileEntity(tileEntity_.pos.add(-1, 0));
	}
}

void IncandescentLampTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	auto source = powerSource_->trait<PowerBufferTrait>();
	float temp = kelvin();

	Joule energyConsumed = 0;
	if (source) {
		Ampere amps = source->voltage() / filamentResistance(temp);
		if (amps > FILAMENT_CURRENT_LIMIT) {
			amps = FILAMENT_CURRENT_LIMIT;
		}

		energyConsumed = source->consume(amps, dt);
	}

	// We need more temporal resolution for Stefan-Boltzmann stuff
	constexpr int N = 4;
	dt /= N;
	energyConsumed /= N;
	Joule totalEnergyEmitted = 0;
	for (int i = 0; i < N; ++i) {
		temperature_ += energyConsumed / FILAMENT_HEAT_CAPACITY;

		constexpr float STEFAN_BOLTZMANN_CONSTANT = 5.670374419e-8;
		float tempHypercubed = temp * temp * temp * temp;

		// M° = σT^4
		float exitance = ( // watts per m^2
			FILAMENT_EMISSIVITY * STEFAN_BOLTZMANN_CONSTANT * tempHypercubed);
		Watt watts = exitance * FILAMENT_AREA;
		Joule energy = watts * dt;
		totalEnergyEmitted += energy;
		temperature_ -= energy / FILAMENT_HEAT_CAPACITY;

		// Make sure we always keep at a reasonable "room temperature".
		if (temperature_ < 0) {
			temperature_ = 0;
		}

		temp = kelvin();
	}

	float light = totalEnergyEmitted * 10;
	if (light > 10) {
		light = 10;
	}
	if (light < 0) {
		light = 0;
	}
	if (temp < 1000) {
		light = 0;
	}

	if (std::abs(light - light_) >= 0.05 || (light == 0 && light_ != 0)) {
		if (light_ > 0) {
			ctx.plane.lights().removeLight(tileEntity_.pos, light_);
		}
		if (light > 0) {
			ctx.plane.lights().addLight(tileEntity_.pos, light);
		}

		light_ = light;
	}
}

void IncandescentLampTileEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (light_ <= 0) {
		return;
	}

	rnd.drawTileSprite({
		.transform = Cygnet::Mat3gf{}.translate(tileEntity_.pos),
		.sprite = glowRedSprite_,
		.opacity = light_ * 2,
	});

	rnd.drawTileSprite({
		.transform = Cygnet::Mat3gf{}.translate(tileEntity_.pos),
		.sprite = glowSprite_,
		.opacity = (light_ - 1) / 2.8f,
	});
}

void IncandescentLampTileEntity::onDespawn(Swan::Ctx &ctx)
{
	if (light_ > 0) {
		ctx.plane.lights().removeLight(tileEntity_.pos, light_);
	}
}

void IncandescentLampTileEntity::drawDebug(Swan::Ctx &ctx)
{
	float voltage = std::numeric_limits<float>::quiet_NaN();
	auto source = powerSource_->trait<PowerBufferTrait>();
	if (source) {
		voltage = source->voltage();
	}

	Ohm resistance = filamentResistance(kelvin());
	Ampere amps = voltage / resistance;

	ImGui::Text("Temperature: %.1fk", temperature_ + 300);
	ImGui::Text("Source voltage: %.1fV", voltage);
	ImGui::Text("Resistance: %.1f Ohm", resistance);
	ImGui::Text("Power: %.1fW", amps * voltage);
	ImGui::Text("Light: %.1f", light_);
}

void IncandescentLampTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	powerSource_.serialize(w.initPowerSource());
}

void IncandescentLampTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	powerSource_.deserialize(ctx, r.getPowerSource());
}

}
