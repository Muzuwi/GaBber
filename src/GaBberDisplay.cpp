#include <thread>
#include <chrono>
#include <fmt/format.h>
#include "Headers/GaBber.hpp"
#include "GL/gl3w.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

bool GaBber::display_initialize() {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
		fmt::print("SDL initialization failed!\n");
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	if(m_debugger.is_debug_mode())
		m_gabberWindow = SDL_CreateWindow("GaBber", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE|SDL_WINDOW_MAXIMIZED);
	else
		m_gabberWindow = SDL_CreateWindow("GaBber", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 240*m_window_scale, 160*m_window_scale, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

	m_gabberGLContext = SDL_GL_CreateContext(m_gabberWindow);
	SDL_GL_MakeCurrent(m_gabberWindow, m_gabberGLContext);

	if(gl3wInit() != 0) {
		fmt::print("Gl3w initialization failed!\n");
		return false;
	}

	auto context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui_ImplSDL2_InitForOpenGL(m_gabberWindow, m_gabberGLContext);
	ImGui_ImplOpenGL3_Init();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 2.0f;
	style.TabRounding = 1.0f;
	style.ScrollbarRounding = 2.0f;
	style.WindowBorderSize = 0.0f;
	style.FrameRounding = 3.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_TextDisabled]           = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
	colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
	colors[ImGuiCol_ChildBg]                = ImVec4(0.07f, 0.07f, 0.07f, 0.00f);
	colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.54f);
	colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.55f, 0.55f, 0.55f, 0.54f);
	colors[ImGuiCol_FrameBgActive]          = ImVec4(0.65f, 0.65f, 0.65f, 0.54f);
	colors[ImGuiCol_TitleBg]                = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBgActive]          = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.86f, 0.36f, 1.00f);
	colors[ImGuiCol_SliderGrab]             = ImVec4(0.04f, 0.95f, 1.00f, 0.40f);
	colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.01f, 0.85f, 0.90f, 0.40f);
	colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.72f, 0.73f, 0.47f);
	colors[ImGuiCol_ButtonHovered]          = ImVec4(0.00f, 0.80f, 0.81f, 0.47f);
	colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 0.91f, 0.92f, 0.47f);
	colors[ImGuiCol_Header]                 = ImVec4(0.57f, 0.57f, 0.57f, 0.31f);
	colors[ImGuiCol_HeaderHovered]          = ImVec4(0.80f, 0.80f, 0.80f, 0.31f);
	colors[ImGuiCol_HeaderActive]           = ImVec4(0.73f, 0.73f, 0.73f, 0.31f);
	colors[ImGuiCol_ResizeGrip]             = ImVec4(0.73f, 0.73f, 0.73f, 0.32f);
	colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.73f, 0.73f, 0.73f, 0.50f);
	colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.73f, 0.73f, 0.73f, 0.78f);
	colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
	colors[ImGuiCol_TabHovered]             = ImVec4(0.00f, 0.77f, 0.78f, 0.47f);
	colors[ImGuiCol_TabActive]              = ImVec4(0.00f, 0.98f, 1.00f, 0.47f);
	colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.81f, 0.47f, 0.35f);
	colors[ImGuiCol_NavHighlight]           = ImVec4(0.15f, 0.72f, 0.54f, 1.00f);
	colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.66f, 0.66f, 0.72f, 0.50f);
	colors[ImGuiCol_SeparatorActive]        = ImVec4(0.45f, 0.45f, 0.51f, 0.50f);
	colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
	colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.03f, 0.70f, 0.71f, 0.47f);

	ImGui::GetIO().Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 18);

	SDL_GL_SetSwapInterval(1);

	_disp_create_gl_state();

	return true;
}

void GaBber::display_update() {
	using hrc = std::chrono::high_resolution_clock;

	static constexpr unsigned target_millis = 1000 / 60;

	static std::optional<hrc::time_point> s_last_drawn;
	if(s_last_drawn.has_value()) {
		auto duration = hrc::now() - *s_last_drawn;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
		if(millis.count() < target_millis) {
			std::this_thread::sleep_for(std::chrono::milliseconds(target_millis - millis.count()));
		}
	}
	_disp_poll_events();
	_disp_draw_frame();
	s_last_drawn = hrc::now();
}

void GaBber::_disp_poll_events() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch (event.type) {
			case SDL_QUIT: m_closed = true; break;
			case SDL_KEYDOWN: {
				switch (event.key.keysym.sym) {
					case SDLK_UP: m_ppu.handle_key_down(KeypadKey::Up); break;
					case SDLK_DOWN: m_ppu.handle_key_down(KeypadKey::Down); break;
					case SDLK_LEFT: m_ppu.handle_key_down(KeypadKey::Left); break;
					case SDLK_RIGHT: m_ppu.handle_key_down(KeypadKey::Right); break;
					case SDLK_z: m_ppu.handle_key_down(KeypadKey::A); break;
					case SDLK_x: m_ppu.handle_key_down(KeypadKey::B); break;
					case SDLK_a: m_ppu.handle_key_down(KeypadKey::Sel); break;
					case SDLK_s: m_ppu.handle_key_down(KeypadKey::Start); break;
					case SDLK_q: m_ppu.handle_key_down(KeypadKey::L); break;
					case SDLK_w: m_ppu.handle_key_down(KeypadKey::R); break;
					default: break;
				}

				if(event.key.keysym.sym == SDLK_F3) {
					single_step();
				}
				if(event.key.keysym.sym == SDLK_F5) {
					resume();
				}
				if(event.key.keysym.sym == SDLK_TAB && event.key.keysym.mod & KMOD_LSHIFT){
					toggle_debug_mode();
				}
				break;
			}
			case SDL_KEYUP: {
				switch (event.key.keysym.sym) {
					case SDLK_UP: m_ppu.handle_key_up(KeypadKey::Up); break;
					case SDLK_DOWN: m_ppu.handle_key_up(KeypadKey::Down); break;
					case SDLK_LEFT: m_ppu.handle_key_up(KeypadKey::Left); break;
					case SDLK_RIGHT: m_ppu.handle_key_up(KeypadKey::Right); break;
					case SDLK_z: m_ppu.handle_key_up(KeypadKey::A); break;
					case SDLK_x: m_ppu.handle_key_up(KeypadKey::B); break;
					case SDLK_a: m_ppu.handle_key_up(KeypadKey::Sel); break;
					case SDLK_s: m_ppu.handle_key_up(KeypadKey::Start); break;
					case SDLK_q: m_ppu.handle_key_up(KeypadKey::L); break;
					case SDLK_w: m_ppu.handle_key_up(KeypadKey::R); break;
					default: break;
				}
				break;
			}
			case SDL_WINDOWEVENT_RESIZED: {
				_disp_create_gl_state();
				break;
			}
			default: break;
		}
	}
}

void GaBber::_disp_draw_frame() {
	glClearColor(0x42 / 255.0, 0x42 / 255.0, 0x42 / 255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//  Update screen texture
	glBindTexture(GL_TEXTURE_2D, m_gba_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 160, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, m_ppu.framebuffer());
	glBindTexture(GL_TEXTURE_2D, 0);

	_disp_draw_common();

	if(!m_debugger.is_debug_mode())
		_disp_draw_gba_screen();
	else
		_disp_draw_debugger();

	int w, h;
	SDL_GetWindowSize(m_gabberWindow, &w, &h);

	glViewport(0, 0, w, h);
	SDL_GL_SwapWindow(m_gabberWindow);
}

void GaBber::_disp_draw_common() {
	//  ImGui toolbar and other cool pop-ups go here
}

void GaBber::_disp_draw_debugger() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_gabberWindow);
	ImGui::NewFrame();
	m_debugger.draw_debugger_contents();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GaBber::_disp_draw_gba_screen() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fb);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_gba_texture, 0);
	glBlitFramebuffer(0, 0, 240, 160,
	                  0, 160*m_window_scale, 240*m_window_scale, 0,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void GaBber::_disp_create_gl_state() {
	glGenFramebuffers(1, &m_fb);
	glGenTextures(1, &m_gba_texture);
	glBindTexture(GL_TEXTURE_2D, m_gba_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}
