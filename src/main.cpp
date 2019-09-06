#include <iostream>
#include <cstdlib>

#include "z80.hpp"
#include "ula.hpp"
#include "bus.hpp"
#include "system.hpp"
#include "debugger.hpp"
#include "options.hpp"

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

	Options options(argc, argv);

	Bus mem(65536);
	Z80 state(mem);
	ULA ula(state, mem);
	Debugger debug(state, mem);

	System sys(state, ula, mem);

	// Use options to set up system
	if (options.rom_on)
	{
		mem.load_rom(options.rom_file);
	}
	
	if (options.break_on)
	{
		debug.set_break(true, options.break_addr);
	}

	bool running = true;

	do
	{
		running = debug.clock();
		running &= sys.clock(debug.is_break_enabled());
	} while (running);

	return EXIT_SUCCESS;
}
