#include <iostream>
#include "Emulator/GaBber.hpp"

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cout << "Usage: <executable> path-to-rom\n";
		return -1;
	}

	GaBber::instance().parse_args(argc, argv);
	return GaBber::instance().start();
}
