#pragma once
#include <string>
#include <functional>
#include <Headers/StdTypes.hpp>
#include "imgui/imgui.h"

class GaBber;
class Debugger {
	GaBber& emu;

	bool m_debug_mode {false};
	bool m_step {false};
	bool m_continue {false};

	void make_window(const std::string& title, ImGuiWindowFlags, std::function<void()>);
public:
	Debugger(GaBber& v)
	: emu(v) {}

	void draw_debugger_contents();

	bool is_step() const { return m_step; }
	void set_step(bool v) { m_step = v; }

	bool continue_mode() const { return m_continue; }
	void set_continue_mode(bool v) { m_continue = v; }

	bool is_debug_mode() const { return m_debug_mode; }
	void set_debug_mode(bool v) { m_debug_mode = v; }

	bool handle_break();
};

