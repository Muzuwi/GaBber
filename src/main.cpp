#include <fmt/format.h>
#include <memory>
#include "Emulator/GaBber.hpp"

int main(int argc, char** argv) {
	auto emulator = std::make_shared<GaBber>();
	if(!emulator->parse_args(argc, argv)) {
		fmt::print("Invalid or not enough arguments specified\n");
		return 1;
	}

	return emulator->start();
}
