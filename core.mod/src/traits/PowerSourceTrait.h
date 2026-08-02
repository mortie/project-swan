#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

/// An ampere represents a current.
/// It is coulombs per second.
/// Since Swan operates on a 20Hz tick system,
/// consuming "1 ampere" is the same as consuming 1/20 coulomb per tick.
using Ampere = float;

/// A farad measures the capacitance of a capacitor.
/// It is equal to one coulomb per volt.
using Farad = float;

/// A coulomb is a unit of electric charge.
/// 1 coulomb is the charge you would consume
/// if you consumed 1 ampere for 1 second.
using Coulomb = float;

/// A joule is a unit of energy.
/// It is equivalent to 1 ampere at 1 volt for 1 second.
using Joule = float;

/// Volt is the energy per electric charge.
using Volt = float;

/// Ohm is a unit of resistance.
/// 1 ohm is the resistance which produces 1 amp of current draw
/// when supplied 1 volt.
using Ohm = float;

/// Watts are amps * volts per second
using Watt = float;

/// A power buffer is pretty much a capacitor.
class PowerSource {
public:
	PowerSource(Farad capacitance): capacitance_(capacitance)
	{}

	/// Compute the current voltage of the power buffer.
	/// A full power buffer will have a voltage equal to its nominal voltage.
	Volt voltage() { return voltage(charge_, capacitance_); }

	// Compute voltage based on a different charge level.
	static Volt voltage(Coulomb charge, Farad capacitance)
	{ return charge / capacitance; }

	Coulomb charge() { return charge_; }
	Farad capacitance() { return capacitance_; }

	/// When you draw power from a power buffer,
	/// you try to draw a certain amperage (coulomb per second).
	/// The amount of energy which that represents depends on the voltage.
	/// The return value is a number of joules which were consumed.
	/// Whatever you do with the power ought to scale linearly
	/// with the number of joules.
	/// You can compute watts by multiplying the joule by 20 (the tick rate in Hz).
	Joule consume(Ampere current, float dt);

	/// Charge up the capacitor with a given target voltage and current.
	/// Returns a scalar representing how much of the available energy
	/// made its way into the capacitor:
	/// 1 if the capacitor was so discharged that everything made its way in,
	/// 0 if the capacitor was alreday at the target voltage,
	/// and anything in between.
	float chargeUp(Ampere current, Volt voltage, float dt);

	/// The power source accumulates charge consumption throughout the tick,
	/// and reconsiliates during the tick2 phase.
	/// This function *must* be called in the owning entity's tick2 function.
	/// In general, power draw should happen during tick(),
	/// and power generation should happen during tick2().
	void tick2();

	void serialize(proto::PowerSource::Builder w);
	void deserialize(proto::PowerSource::Reader r);

	void drawDebug();

private:
	Farad capacitance_ = 0;
	Coulomb charge_ = 0;
	Coulomb chargeConsumedThisTick_ = 0;
	Ampere currentDraw_ = 0;
};

class PowerSourceTrait {
public:
	struct Tag {};

	virtual PowerSource &get(Tag) = 0;

protected:
	~PowerSourceTrait() = default;
};

}
