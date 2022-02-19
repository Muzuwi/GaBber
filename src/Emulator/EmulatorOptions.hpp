#pragma once
#include "Emulator/Module.hpp"

class EmulatorOptions : Module {
public:
	EmulatorOptions(GaBber& emu)
	    : Module(emu) {}

	void draw();
};