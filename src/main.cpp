#include <iostream>
#include <cstdlib>

#include "z80.hpp"

static unsigned short break_pc = 0xffff;

/**
 * Should we enable a debug break?
 */
static bool enable_break(Z80 &state)
{
	if (state.pc.get() == break_pc)
	{
		return true;
	}

	return false;
}

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <binary image>" << std::endl;
		return EXIT_FAILURE;
	}

	if (argc >= 3)
	{
		break_pc = atoi(argv[2]);
	}

	std::string rom_file(argv[1]);
	Z80 state(65536, rom_file);

	bool running = true;
	bool do_break = false;
	int step = 0;

	do
	{
		if (!do_break && enable_break(state))
		{
			do_break = true;
		}

		if (!step && do_break)
		{
			char ch;
			std::cin >> ch;

			switch (ch)
			{
			case 'c':
				do_break = false;
				break;
			case 's':
				std::cin >> step;
				break;
			case 'd':
				state.dump();
				break;
			case 'q':
				running = false;
				break;
			}
		}

		if (step)
		{
			step--;
		}

	} while (running && state.clock());

	return EXIT_SUCCESS;
}
