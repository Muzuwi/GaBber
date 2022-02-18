#pragma once
#include <imgui.h>
#include <string>
#include "Emulator/Module.hpp"

class DebuggerWindow : public Module {
protected:
	std::string m_name;
	bool m_is_open;
	ImGuiWindowFlags m_flags;

	virtual void draw_window() {}
public:
	DebuggerWindow(std::string name, GaBber& emu)
	    : Module(emu)
	    , m_name(std::move(name))
	    , m_is_open(true)
	    , m_flags(0) {}

	void draw();
};
