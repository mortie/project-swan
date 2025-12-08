#pragma once

#include "imgui/imgui.h"

inline void StyleColors() {
	ImGui::StyleColorsDark();

	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding            = ImVec2(15, 15);
	style->WindowRounding           = 5.0f;
	style->FramePadding             = ImVec2(5, 5);
	style->FrameRounding            = 4.0f;
	style->ItemSpacing              = ImVec2(12, 8);
	style->ItemInnerSpacing         = ImVec2(8, 6);
	style->IndentSpacing            = 25.0f;
	style->ScrollbarSize            = 15.0f;
	style->ScrollbarRounding        = 9.0f;
	style->GrabMinSize              = 5.0f;
	style->GrabRounding             = 3.0f;
}
