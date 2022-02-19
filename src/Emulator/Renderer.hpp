#pragma once
#include <GL/glew.h>
#include <SDL.h>
#include "Emulator/AudioOptions.hpp"
#include "Emulator/Module.hpp"
#include "ShellFlags.hpp"

class GaBber;

class Renderer : Module {
	SDL_Window* m_window {};
	SDL_GLContext m_gl_context {};
	float m_last_frame_time { 0.001f };
	unsigned m_window_scale { 5 };
	GLuint m_fb {};
	GLuint m_screen_texture {};
	ShellFlags m_shell_flags {};
	AudioOptions m_audio_options_window;

	void render_ui_common();
	void render_gba_screen();
	void render_debugger();
	void render_frame();
	void create_gl_state();
	void poll_events();
public:
	Renderer(GaBber&);

	bool initialize_platform();
	void update();

	void resize_to_debugger();
	void resize_to_normal();

	GLuint gba_texture() const { return m_screen_texture; }
	unsigned window_scale() const { return m_window_scale; }
};