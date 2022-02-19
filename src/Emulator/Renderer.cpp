#include "Renderer.hpp"
#include <chrono>
#include <fmt/format.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <thread>
#include "Bus/IO/Keypad.hpp"
#include "Debugger/Debugger.hpp"
#include "Emulator/GaBber.hpp"
#include "PPU/PPU.hpp"

Renderer::Renderer(GaBber& emu)
    : Module(emu)
    , m_audio_options_window(emu) {}

bool Renderer::initialize_platform() {
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
		fmt::print("SDL initialization failed!\n");
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	if(debugger().is_debug_mode()) {
		m_window = SDL_CreateWindow("GaBber", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720,
		                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);
	} else {
		m_window = SDL_CreateWindow("GaBber", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 240 * m_window_scale,
		                            160 * m_window_scale, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	}

	m_gl_context = SDL_GL_CreateContext(m_window);
	SDL_GL_MakeCurrent(m_window, m_gl_context);

	auto status = glewInit();
	if(status != GLEW_OK) {
		fmt::print("GLEW initialization failed!\n");
		fmt::print("Error: {}\n", glewGetErrorString(status));
		return false;
	}

	auto context = ImGui::CreateContext();
	ImGui::SetCurrentContext(context);
	ImGui_ImplSDL2_InitForOpenGL(m_window, m_gl_context);
	ImGui_ImplOpenGL3_Init();

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 2.0f;
	style.TabRounding = 1.0f;
	style.ScrollbarRounding = 2.0f;
	style.WindowBorderSize = 0.0f;
	style.FrameRounding = 3.0f;

	ImVec4* colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.07f, 0.07f, 0.07f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.55f, 0.55f, 0.55f, 0.54f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.65f, 0.65f, 0.65f, 0.54f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.86f, 0.36f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.04f, 0.95f, 1.00f, 0.40f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.01f, 0.85f, 0.90f, 0.40f);
	colors[ImGuiCol_Button] = ImVec4(0.00f, 0.72f, 0.73f, 0.47f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.80f, 0.81f, 0.47f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.91f, 0.92f, 0.47f);
	colors[ImGuiCol_Header] = ImVec4(0.57f, 0.57f, 0.57f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.80f, 0.80f, 0.80f, 0.31f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.73f, 0.73f, 0.73f, 0.31f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.73f, 0.73f, 0.73f, 0.32f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.73f, 0.73f, 0.73f, 0.50f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.73f, 0.73f, 0.73f, 0.78f);
	colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.77f, 0.78f, 0.47f);
	colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.98f, 1.00f, 0.47f);
	colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 0.81f, 0.47f, 0.35f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.15f, 0.72f, 0.54f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.66f, 0.66f, 0.72f, 0.50f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.45f, 0.51f, 0.50f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.03f, 0.70f, 0.71f, 0.47f);

	ImGui::GetIO().Fonts->AddFontFromFileTTF("Roboto-Medium.ttf", 18);

	SDL_GL_SetSwapInterval(0);

	create_gl_state();

	return true;
}

void Renderer::create_gl_state() {
	glGenFramebuffers(1, &m_fb);
	glGenTextures(1, &m_screen_texture);
	glBindTexture(GL_TEXTURE_2D, m_screen_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Renderer::render_gba_screen() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fb);
	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_screen_texture, 0);
	glBlitFramebuffer(0, 0, 240, 160, 0, 160 * m_window_scale, 240 * m_window_scale, 0, GL_COLOR_BUFFER_BIT,
	                  GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

void Renderer::render_debugger() {
	debugger().draw_debugger_contents();
}

void Renderer::render_ui_common() {
	if(ImGui::BeginMainMenuBar()) {
		if(ImGui::BeginMenu("File")) {
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Debugger")) {
			ImGui::EndMenu();
		}
		if(ImGui::BeginMenu("Options")) {
			if(ImGui::MenuItem("Audio")) {
				m_shell_flags.audio_options_open = true;
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if(m_shell_flags.audio_options_open) {
		ImGui::Begin("Audio", &m_shell_flags.audio_options_open,
		             ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse);
		m_audio_options_window.draw();
		ImGui::End();
	}
}

void Renderer::render_frame() {
	glClearColor(0x42 / 255.0, 0x42 / 255.0, 0x42 / 255.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	//  Update screen texture
	glBindTexture(GL_TEXTURE_2D, m_screen_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 240, 160, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ppu().framebuffer());
	glBindTexture(GL_TEXTURE_2D, 0);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(m_window);
	ImGui::NewFrame();

	render_ui_common();

	if(!debugger().is_debug_mode()) {
		render_gba_screen();
	} else {
		render_debugger();
	}

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	int w, h;
	SDL_GetWindowSize(m_window, &w, &h);

	glViewport(0, 0, w, h);
	SDL_GL_SwapWindow(m_window);
}

void Renderer::update() {
	using hrc = std::chrono::high_resolution_clock;
	static std::optional<hrc::time_point> s_last_drawn;

	const int64 target_micros = 1000000 / config().target_framerate;
	if(s_last_drawn.has_value()) {
		auto duration = hrc::now() - *s_last_drawn;
		auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);
		if(micros.count() < target_micros) {
			std::this_thread::sleep_for(std::chrono::microseconds(target_micros - micros.count()));
		}

		const auto now = hrc::now();
		const auto real_frame_duration = now - *s_last_drawn;
		const auto real_frame_micros = std::chrono::duration_cast<std::chrono::microseconds>(real_frame_duration);
		m_last_frame_time = (float)real_frame_micros.count() / 1000000.0f;
	}

	auto str = fmt::format("GaBber - {:.1f} FPS", 1.0f / m_last_frame_time);
	SDL_SetWindowTitle(m_window, str.c_str());
	s_last_drawn = hrc::now();

	poll_events();
	render_frame();
}

void Renderer::poll_events() {
	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		ImGui_ImplSDL2_ProcessEvent(&event);
		switch(event.type) {
			case SDL_QUIT: {
				m_emu.close();
				break;
			}
			case SDL_KEYDOWN: {
				switch(event.key.keysym.sym) {
					case SDLK_UP: ppu().handle_key_down(KeypadKey::Up); break;
					case SDLK_DOWN: ppu().handle_key_down(KeypadKey::Down); break;
					case SDLK_LEFT: ppu().handle_key_down(KeypadKey::Left); break;
					case SDLK_RIGHT: ppu().handle_key_down(KeypadKey::Right); break;
					case SDLK_z: ppu().handle_key_down(KeypadKey::A); break;
					case SDLK_x: ppu().handle_key_down(KeypadKey::B); break;
					case SDLK_a: ppu().handle_key_down(KeypadKey::Sel); break;
					case SDLK_s: ppu().handle_key_down(KeypadKey::Start); break;
					case SDLK_q: ppu().handle_key_down(KeypadKey::L); break;
					case SDLK_w: ppu().handle_key_down(KeypadKey::R); break;
					default: break;
				}

				if(event.key.keysym.sym == SDLK_F3) {
					m_emu.single_step();
				}
				if(event.key.keysym.sym == SDLK_F5) {
					m_emu.resume();
				}
				if(event.key.keysym.sym == SDLK_TAB && event.key.keysym.mod & KMOD_LSHIFT) {
					m_emu.toggle_debug_mode();
				}
				break;
			}
			case SDL_KEYUP: {
				switch(event.key.keysym.sym) {
					case SDLK_UP: ppu().handle_key_up(KeypadKey::Up); break;
					case SDLK_DOWN: ppu().handle_key_up(KeypadKey::Down); break;
					case SDLK_LEFT: ppu().handle_key_up(KeypadKey::Left); break;
					case SDLK_RIGHT: ppu().handle_key_up(KeypadKey::Right); break;
					case SDLK_z: ppu().handle_key_up(KeypadKey::A); break;
					case SDLK_x: ppu().handle_key_up(KeypadKey::B); break;
					case SDLK_a: ppu().handle_key_up(KeypadKey::Sel); break;
					case SDLK_s: ppu().handle_key_up(KeypadKey::Start); break;
					case SDLK_q: ppu().handle_key_up(KeypadKey::L); break;
					case SDLK_w: ppu().handle_key_up(KeypadKey::R); break;
					default: break;
				}
				break;
			}
			case SDL_WINDOWEVENT_RESIZED: {
				create_gl_state();
				break;
			}
			default: break;
		}
	}
}

void Renderer::resize_to_debugger() {
	SDL_SetWindowResizable(m_window, SDL_FALSE);
	SDL_RestoreWindow(m_window);
}

void Renderer::resize_to_normal() {
	SDL_SetWindowResizable(m_window, SDL_TRUE);
	SDL_MaximizeWindow(m_window);
}
