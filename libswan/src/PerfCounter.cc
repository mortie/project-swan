#include "PerfCounter.h"

#include <imgui.h>

#include "util.h"

namespace Swan {

void PerfCounter::render() {
	Deferred win([]{ ImGui::End(); });
	if (!ImGui::Begin("Perf Stats"))
		return;

	ImGui::PushItemWidth(ImGui::GetFontSize() * -12);
	ImGui::Text("Hello World");

	std::array<float, 64> buf;
	frame_time_.fill(buf);
	ImGui::PlotLines("Frame Time", buf.data(), 64);
}

}
