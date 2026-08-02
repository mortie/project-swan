#include "PowerNodeTrait.h"
#include "traits/PowerSourceTrait.h"

namespace CoreMod {

void PowerNode::onDespawn(Swan::Ctx &ctx)
{
	std::unordered_set<PowerNode *> seen;
	invalidateNetwork(seen);

	for (auto &wire: wires_) {
		ctx.plane.entities().despawn(wire);
	}
}

void PowerNode::attach(Swan::EntityRef wire)
{
	wires_.push_back(wire);
}

void PowerNode::serialize(proto::PowerNode::Builder w)
{
	auto wires = w.initWires(wires_.size());
	for (size_t i = 0; i < wires_.size(); ++i) {
		wires_[i].serialize(wires[i]);
	}
}

void PowerNode::deserialize(Swan::Ctx &ctx, proto::PowerNode::Reader r)
{
	powerSource_.reset();
	auto wires = r.getWires();
	wires_.resize(wires.size());
	for (size_t i = 0; i < wires.size(); ++i) {
		wires_[i].deserialize(ctx, wires[i]);
	}
}

Swan::EntityRef PowerNode::powerSource()
{
	if (powerSource_.has_value()) {
		return *powerSource_;
	}

	std::unordered_set<PowerNode *> seen;
	powerSource_ = findPowerSource(seen);
	return *powerSource_;
}

void PowerNode::invalidateNetwork()
{
	std::unordered_set<PowerNode *> seen;
	invalidateNetwork(seen);
}

Swan::EntityRef PowerNode::findPowerSource(std::unordered_set<PowerNode *> &seen)
{
	for (size_t i = 0; i < wires_.size();) {
		auto wireRef = wires_[i];
		auto wire = wireRef.as<CopperWireEntity>();
		if (!wire) {
			wires_[i] = wires_.back();
			wires_.pop_back();
			continue;
		}
		i += 1;

		 for (auto ref: {wire->begin_, wire->end_}) {
			if (ref.as<PowerSourceTrait>()) {
				return ref;
			}

			auto node = ref.trait<PowerNodeTrait>();
			if (!node || seen.contains(node)) {
				continue;
			}

			seen.insert(node);
			auto powerSource = node->findPowerSource(seen);
			if (powerSource) {
				return powerSource;
			}
		}
	}

	return {};
}

void PowerNode::invalidateNetwork(std::unordered_set<PowerNode *> &seen)
{
	powerSource_.reset();

	for (size_t i = 0; i < wires_.size();) {
		auto wireRef = wires_[i];
		auto wire = wireRef.as<CopperWireEntity>();
		if (!wire) {
			wires_[i] = wires_.back();
			wires_.pop_back();
			continue;
		}
		i += 1;

		 for (auto ref: {wire->begin_, wire->end_}) {
			auto node = ref.trait<PowerNodeTrait>();
			if (!node || seen.contains(node)) {
				continue;
			}

			seen.insert(node);
			node->invalidateNetwork(seen);
		}
	}
}

}
