#include "PerfCounter.h"

#include <imgui.h>
#include <imgui_plot.h>

#include "util.h"

namespace Swan {

void PerfCounter::render() {
	Deferred win([]{ ImGui::End(); });
	if (!ImGui::Begin("Perf Stats"))
		return;

	std::array<float, 64> buf;
	ImGui::PlotConfig conf;

	conf.values = { .ys = buf.data(), .count = 64 };
	conf.scale = { 0, 1 / 60.0 };
	conf.scale.min = 0;
	conf.scale.max = 1 / 60.0;
	conf.frame_size = { ImGui::GetWindowContentRegionWidth(), 20 },

	total_time_.fill(buf);
	ImGui::Text("Total Time");
	ImGui::Plot("Total Time", conf);

	render_present_.fill(buf);
	ImGui::Text("Render Present");
	ImGui::Plot("Render Present", conf);

	frame_time_.fill(buf);
	ImGui::Text("Frame Times");
	ImGui::Plot("Frame Times", conf);

	game_update_.fill(buf);
	ImGui::Text("Game Update");
	ImGui::Plot("Game Update", conf);

	game_draw_.fill(buf);
	ImGui::Text("Game Draw");
	ImGui::Plot("Game Draw", conf);

	game_tick_.fill(buf);
	ImGui::Text("Game Tick");
	ImGui::Plot("Game Tick", conf);

	game_updates_per_frame_.fill(buf);
	conf.scale = { 0, 5 };
	ImGui::Text("Game Updates Per Frame");
	ImGui::Plot("Game Updates Per Frame", conf);
}

}
