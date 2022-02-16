#pragma once
#include <imgui.h>
#include <string>

class GaBber;
class ARM7TDMI;

class DebuggerWindow {
protected:
	std::string m_name;
	bool m_is_open;
	ImGuiWindowFlags m_flags;
	GaBber& m_emu;

	ARM7TDMI& cpu();

	virtual void draw_window() {}
public:
	DebuggerWindow(std::string name, GaBber& emu)
	    : m_name(std::move(name))
	    , m_is_open(true)
	    , m_flags(0)
	    , m_emu(emu) {}

	void draw();
};
