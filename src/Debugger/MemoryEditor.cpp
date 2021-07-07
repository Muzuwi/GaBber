#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"
#include "MMU/MMU.hpp"

void MemEditor::draw_window() {
	static MMU& mmu = this->m_emu.mmu();

	m_editor.ReadFn = [](const ImU8* addr, size_t off) -> ImU8 {
		return mmu.peek((uint64)addr + off);
	};
	m_editor.WriteFn = [](ImU8* addr, size_t off, ImU8 val) {
		mmu.poke((uint64)addr + off, val);
	};

	m_editor.DrawContents(nullptr, 0xfffffff);
}

