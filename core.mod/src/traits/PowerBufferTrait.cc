#include "PowerBufferTrait.h"

namespace CoreMod {

Joule PowerBuffer::consume(Ampere current, float dt)
{
	Coulomb delta = current * dt;

	// Only allow drawing up to 1/10 of the buffer at a time.
	// This represents ESR I guess?
	// It also makes tick order less obviously significant
	// if multiple things consume from the same power buffer at a time.
	if (delta > charge_ / 10) {
		delta = charge_ / 10;
	}

	// As a special case, only allow 'charge_ - chargeConsumedThisTick_'
	// to become 0, never negative.
	// This violates fairness but avoids breaking the capacitor :)
	if (charge_ - chargeConsumedThisTick_ - delta < 0) {
		delta = charge_ - chargeConsumedThisTick_;
	}

	// Consume the charge,
	// and use the average voltage of before and after
	// to compute the joules
	Volt before = voltage(charge_, capacitance_);
	chargeConsumedThisTick_ += delta;
	Volt after = voltage(charge_ - delta, capacitance_);
	return delta * ((before + after) / 2);
}

float PowerBuffer::chargeUp(Ampere current, Volt voltage, float dt)
{
	Coulomb delta = current * dt;

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

void PowerBuffer::tick2()
{
	charge_ -= chargeConsumedThisTick_;
	currentDraw_ = chargeConsumedThisTick_ * Swan::TICK_RATE;
	chargeConsumedThisTick_ = 0;
}

void PowerBuffer::serialize(proto::PowerBuffer::Builder w)
{
	w.setCharge(charge_);
}

void PowerBuffer::deserialize(proto::PowerBuffer::Reader r)
{
	charge_ = r.getCharge();
}

void PowerBuffer::drawDebug()
{
	ImGui::Text("Voltage: %s", Swan::siPrefix(voltage(), "V"));
	ImGui::Text("Charge: %s", Swan::siPrefix(charge_, "C"));
	ImGui::Text("Capacitance: %s", Swan::siPrefix(capacitance_, "F"));
	ImGui::Text("Current draw: %s", Swan::siPrefix(currentDraw_, "A"));
}

}
