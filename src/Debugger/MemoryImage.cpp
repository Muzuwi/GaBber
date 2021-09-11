#include "GL/gl3w.h"
#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"

void MemoryImage::draw_window() {
	if(!created_tex) {
		GLuint newTex;
		glGenTextures(1, &newTex);
		tex = newTex;
		created_tex = true;
	}

	auto const* ptr = m_emu.mem().onchip_wram.raw();
    auto size = m_emu.mem().onchip_wram.bytes();

	const unsigned width = 128;
	const unsigned height = size / (width * 4);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, ptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindTexture(GL_TEXTURE_2D, 0);

	ImGui::Image((ImTextureID)tex, ImVec2(width*4, height*4));
}