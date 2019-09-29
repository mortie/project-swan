#pragma once

#include <swan/swan.h>

class EntItemStack: public Swan::Entity {
public:
	class Factory: public Swan::Entity::Factory {
		Swan::Entity *create(const Swan::Context &ctx, const Swan::SRF &params) override {
			return new EntItemStack(ctx, params);
		}
	};

	EntItemStack(const Swan::Context &ctx, const Swan::SRF &params);

	void readSRF(const Swan::Context &ctx, const Swan::SRF &srf) override;
	Swan::SRF *writeSRF(const Swan::Context &ctx) override;

private:
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.5, 0.5);

	Swan::Item *item_ = &Swan::Item::INVALID_ITEM;
	Swan::Body body_;
};
