#include "Bus/Common/BusInterface.hpp"
#include "CPU/ARM7TDMI.hpp"
#include "Debugger/WindowDefinitions.hpp"
#include "Emulator/GaBber.hpp"

void Stacktrace::draw_window() {
	static BusInterface& bus = this->bus();

	m_stack.ReadFn = [](const ImU8* addr, size_t off) -> ImU8 { return bus.peek((uint64)addr + off); };
	m_stack.WriteFn = [](ImU8* addr, size_t off, ImU8 val) { bus.poke((uint64)addr + off, val); };

	const auto current_sp = cpu().sp();
	m_stack.OptShowDataPreview = true;
	m_stack.OptGreyOutZeroes = true;
	m_stack.OptMidColsCount = 4;
	m_stack.Cols = 4;
	m_stack.PreviewDataType = ImGuiDataType_U32;
	m_stack.OptShowOptions = false;
	m_stack.DrawContents((void*)current_sp, 0x03007f00u - current_sp);
}
