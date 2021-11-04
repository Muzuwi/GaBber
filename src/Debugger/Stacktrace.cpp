#include "Debugger/WindowDefinitions.hpp"
#include "Headers/GaBber.hpp"

void Stacktrace::draw_window() {
	m_stack.ReadFn = [](const ImU8* addr, size_t off) -> ImU8 {
		return GaBber::instance().mmu().peek((uint64)addr + off);
	};
	m_stack.WriteFn = [](ImU8* addr, size_t off, ImU8 val) {
		GaBber::instance().mmu().poke((uint64)addr + off, val);
	};

	const auto current_sp = GaBber::instance().cpu().sp();
	m_stack.OptShowDataPreview = true;
	m_stack.OptGreyOutZeroes = true;
	m_stack.OptMidColsCount = 4;
	m_stack.Cols = 4;
	m_stack.PreviewDataType = ImGuiDataType_U32;
	m_stack.OptShowOptions = false;
	m_stack.DrawContents((void*)current_sp, 0x03007f00u - current_sp);
}
