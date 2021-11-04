#pragma once

enum BreakpointType {
	BreakRead  = 0x01,
	BreakWrite = 0x02,
	BreakExec  = 0x04
};

struct Breakpoint {
	uint32 start;
	uint32 size;
	BreakpointType type;

	Breakpoint() = default;

	Breakpoint(uint32 start_, uint32 size_, BreakpointType type_)
	: start(start_), size(size_), type(type_) {}
};

struct MemoryEvent {
	uint32 address;
	uint32 value;
	uint32 size;
	BreakpointType type;

	MemoryEvent(uint32 addr, uint32 val, uint32 size_, BreakpointType event_type)
	: address(addr), value(val), size(size_), type(event_type) {}
};

