#pragma once

#include "swan/constants.h"
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

/// A power buffer is pretty much a capacitor.
class PowerBuffer {
public:
	/// Compute the current voltage of the power buffer.
	/// A full power buffer will have a voltage equal to its nominal voltage.
	Volt voltage() { return charge_ / capacitance_; }

	/// When you draw power from a power buffer,
	/// you try to draw a certain amperage (coulomb per second).
	/// The amount of energy which that represents depends on the voltage.
	/// The return value is a number of joules which were consumed.
	/// Whatever you do with the power ought to scale linearly
	/// with the number of joules.
	/// This function is assumed to be called every tick that you draw current;
	/// as a result, amps are converted into coulombs by a 1/20 factor.
	Joule consume(Ampere current)
	{
		Coulomb delta = current / Swan::TICK_RATE;

		// Only allow drawing up to 1/10 of the buffer at a time.
		// This represents ESR I guess?
		// It also makes tick order less obviously significant
		// if multiple things consume from the same power buffer at a time.
		if (delta > charge_ / 10) {
			delta = charge_ / 10;
		}

		// Consume the charge,
		// and use the average voltage of before and after
		// to compute the joules
		Volt before = voltage();
		charge_ -= delta;
		Volt after = voltage();
		return delta * ((before + after) / 2);
	}

	/// Charge up the capacitor with a given target voltage and current.
	/// Returns a scalar representing how much of the available energy
	/// made its way into the capacitor:
	/// 1 if the capacitor was so discharged that everything made its way in,
	/// 0 if the capacitor was alreday at the target voltage,
	/// and anything in between.
	float chargeUp(Ampere current, Volt voltage)
	{
		Coulomb delta = current / Swan::TICK_RATE;

		Coulomb maxDelta = voltage * capacitance_ - charge_;
		if (maxDelta <= 0) {
			return 0;
		}

		float fraction = 1.0;
		if (delta > maxDelta) {
			fraction = maxDelta / delta;
			delta = maxDelta;
		}

		charge_ += delta;
		return fraction;
	}

private:
	Farad capacitance_ = 1;
	Coulomb charge_ = 0;
};

class PowerBufferTrait {
	struct Tag {};

	virtual PowerBuffer &get(Tag) = 0;

protected:
	~PowerBufferTrait() = default;
};

}
