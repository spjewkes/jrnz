#include <iostream>
#include <cstdlib>

#include "z80.hpp"
#include "memory.hpp"
#include "system.hpp"
#include "debugger.hpp"

static uint16_t break_pc = 0xffff;

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <binary image> [<break pc>]" << std::endl;
		return EXIT_FAILURE;
	}

	std::string rom_file(argv[1]);
	Memory mem(65536, rom_file);
	Z80 state(mem);

	// Set up debugging class
	Debugger debug(state, mem);
	if (argc >= 3)
	{
		break_pc = strtoul(argv[2], NULL, 0);
		debug.set_break(true, break_pc);
	}

	System sys(state, mem);

	bool running = true;

	do
	{
		running = debug.clock();
		running &= sys.clock(debug.is_break_enabled());
	} while (running);

	return EXIT_SUCCESS;
}
