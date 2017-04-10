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
		std::cout << "Enabled break at 0x" << std::hex << break_pc << std::dec << std::endl;
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
		std::cerr << "Usage: " << argv[0] << " <binary image> [<break pc>]" << std::endl;
		return EXIT_FAILURE;
	}

	if (argc >= 3)
	{
		break_pc = strtoul(argv[2], NULL, 0);
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
			bool debug = true;
			char ch;

			while (debug)
			{
				std::cout << "Executing: " << state.dump_instr_at_pc(state.pc.get()).str() << std::endl;
				std::cin >> ch;

				switch (ch)
				{
				case 'c':
					do_break = false;
					debug = false;
					break;
				case 's':
					std::cin >> step;
					debug = false;
					break;
				case 'r':
					state.dump();
					break;
				case 't':
					state.dump_sp();
					break;
				case 'd':
					size_t offset;
					size_t size;
					std::cin >> offset >> size;
					std::cout << state.mem.dump(offset, size) << std::endl;
					break;
				case 'n':
					debug = false;
					break;
				case 'q':
					running = false;
					debug = false;
					break;
				default:
					std::string help_text =
						"In debug mode.\n"
						"Help:\n"
						"\tc = continue\n"
						"\ts <n> = step <n> times\n"
						"\tr = dump registers\n"
						"\tt = dump stack\n"
						"\td <offset> <size> = dump memory contents starting at <offset> for <size> bytes\n"
						"\tn = next instruction\n"
						"\tq = quit\n";
					std::cout << help_text;
					break;
				}
			}
		}

		if (step)
		{
			step--;
		}

	} while (running && state.clock());

	return EXIT_SUCCESS;
}
