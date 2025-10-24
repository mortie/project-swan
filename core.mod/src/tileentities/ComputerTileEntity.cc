#include "ComputerTileEntity.h"
#include "imgui/imgui.h"

#include <scisa/scisasm.h>
#include <sstream>

namespace CoreMod {

void ComputerTileEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (!showGUI_) {
		return;
	}

	ImGui::Begin("Computer", &showGUI_);
	ImGui::Text("Assembly Code");
	ImGui::InputTextMultiline("##Assembly", &assembly_, {-1, 300});

	if (ImGui::Button("Assemble")) {
		assemble();
	}

	ImGui::SameLine();

	if (running_) {
		if (ImGui::Button("Stop")) {
			running_ = false;
		}
	} else {
		ImGui::BeginDisabled(asmError_ != "" || textSize_ == 0);
		if (ImGui::Button("Run")) {
			running_ = true;
		}
		ImGui::EndDisabled();
	}

	ImGui::SameLine();

	ImGui::BeginDisabled(asmError_ != "" || textSize_ == 0 || running_);
	if (ImGui::Button("Step")) {
		cpu_.step(1);
	}
	ImGui::EndDisabled();

	ImGui::SameLine();

	if (ImGui::Button("Reset")) {
		cpu_.pc = 0;
		cpu_.sp = 128;
		cpu_.acc = 0;
		cpu_.x = 0;
		cpu_.y = 0;
		debugIO_->output.clear();
	}

	if (asmError_ != "") {
		ImGui::TextColored({255, 0, 0, 255}, "ASM: %s", asmError_.c_str());
	} else {
		ImGui::Text("ASM OK");
	}

	if (cpu_.error) {
		ImGui::TextColored({255, 0, 0, 255}, "CPU: %s", cpu_.error);
	} else {
		ImGui::Text("CPU OK");
	}

	ImGui::Text("TEXT %d DATA %d", textSize_, dataSize_);

	ImGui::Separator();

	ImGui::Text(
		"PC %03d SP %03d ACC %03d X %03d Y %03d",
		cpu_.pc, cpu_.sp, cpu_.acc, cpu_.x, cpu_.y);

	ImGui::BeginDisabled();
	ImGui::PushItemWidth(-1);
	ImGui::InputText("##Output", &debugIO_->output);
	ImGui::PopItemWidth();
	ImGui::EndDisabled();

	ImGui::End();
}

void ComputerTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	if (!running_) {
		return;
	}

	if (cpu_.error) {
		running_ = false;
		return;
	}

	cpu_.step(1);
}

void ComputerTileEntity::assemble()
{
	std::stringstream in(std::move(assembly_));

	scisasm::Assembly a;
	asmError_ = "";
	if (scisasm::assemble(in, a, &asmError_) < 0) {
		assembly_ = std::move(in).str();
		return;
	}

	if (scisasm::link(a, &asmError_) < 0) {
		assembly_ = std::move(in).str();
		return;
	}

	pmem_ = std::move(a.text.content);
	textSize_ = pmem_.size();
	pmem_.resize(256);
	cpu_.pmem = pmem_;

	dmem_ = std::move(a.data.content);
	dataSize_ = dmem_.size();
	pmem_.resize(256);
	cpu_.dmem.clear();
	cpu_.dmem.push_back({
		.data = dmem_,
		.start = 0,
	});
	cpu_.io.push_back({
		.io = debugIO_.get(),
		.start = 255,
		.size = 1,
	});

	asmError_ = "";
	assembly_ = std::move(in).str();
}

}
