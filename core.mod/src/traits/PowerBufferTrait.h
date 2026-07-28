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
class PowerBuffer {
public:
	/// Compute the current voltage of the power buffer.
	/// A full power buffer will have a voltage equal to its nominal voltage.
	Volt voltage() { return charge_ / capacitance_; }

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

	void serialize(proto::PowerBuffer::Builder w);
	void deserialize(proto::PowerBuffer::Reader r);

private:
	Farad capacitance_ = 0.2;
	Coulomb charge_ = 0;
};

class PowerBufferTrait {
public:
	struct Tag {};

	virtual PowerBuffer &get(Tag) = 0;

protected:
	~PowerBufferTrait() = default;
};

}
