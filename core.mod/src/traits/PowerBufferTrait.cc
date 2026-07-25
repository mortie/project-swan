#include "PowerBufferTrait.h"

namespace CoreMod {

Joule PowerBuffer::consume(Ampere current)
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

float PowerBuffer::chargeUp(Ampere current, Volt voltage)
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

void PowerBuffer::serialize(proto::PowerBuffer::Builder w)
{
	w.setCharge(charge_);
}

void PowerBuffer::deserialize(proto::PowerBuffer::Reader r)
{
	charge_ = r.getCharge();
}

}
