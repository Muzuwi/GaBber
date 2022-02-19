#include "Emulator/EmulatorOptions.hpp"
#include <imgui.h>
#include "Emulator/Config.hpp"

void EmulatorOptions::draw() {
	ImGui::InputScalar("Framerate", ImGuiDataType_U32, &config().target_framerate);
}
