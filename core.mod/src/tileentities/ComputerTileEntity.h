#pragma once

#include <swan/swan.h>
#include <scisa/scisavm.h>
#include <vector>

#include "core_mod.capnp.h"

namespace CoreMod {

class ComputerTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait {
public:
	using Proto = proto::ComputerTileEntity;

	ComputerTileEntity(Swan::Ctx &)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void tick(Swan::Ctx &ctx, float dt) override;
	void activate() { showGUI_ = true; }

	void serialize(Swan::Ctx &ctx, Proto::Builder w) {}
	void deserialize(Swan::Ctx &ctx, Proto::Reader r) {}

private:
	struct TextIO: public scisavm::MemoryIO {
		std::string output;

		void store(size_t, uint8_t data) override
		{
			output.push_back(char(data));
		}
	};

	void assemble();

	TileEntity tileEntity_;
	bool running_ = false;
	bool showGUI_ = false;
	std::string asmError_;
	std::string assembly_;

	scisavm::CPU8 cpu_;
	std::vector<uint8_t> pmem_;
	std::vector<uint8_t> dmem_;

	int textSize_ = 0;
	int dataSize_ = 0;
	std::unique_ptr<TextIO> debugIO_ = std::make_unique<TextIO>();
};

}
