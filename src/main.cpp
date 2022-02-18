#include <iostream>
#include <memory>
#include "Emulator/GaBber.hpp"

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cout << "Usage: <executable> path-to-rom\n";
		return -1;
	}

	auto emulator = std::make_shared<GaBber>();
	emulator->parse_args(argc, argv);
	return emulator->start();
}
