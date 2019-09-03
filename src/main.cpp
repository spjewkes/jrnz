#include <iostream>
#include <cstdlib>

#include "z80.hpp"
#include "memory.hpp"
#include "system.hpp"

static uint16_t break_pc = 0xffff;

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
	Memory mem(65536, rom_file);
	Z80 state(mem);
	System sys(state, mem);

	bool running = true;
	bool do_break = false;
	int step = 0;

	do
	{
		if (!do_break && enable_break(sys.z80()))
		{
			do_break = true;
		}

		if (!step && do_break)
		{
			bool debug = true;
			char ch;

			while (debug)
			{
				std::cout << "Executing: " << sys.z80().dump_instr_at_pc(sys.z80().pc.get()).str() << std::endl;
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
					sys.z80().dump();
					break;
				case 't':
					sys.z80().dump_sp();
					break;
				case 'd':
				{
					std::string str_offset;
					std::string str_size;
					std::cin >> str_offset >> str_size;
					size_t offset = strtoul(str_offset.c_str(), NULL, 0);
					size_t size = strtoul(str_size.c_str(), NULL, 0);
					std::cout << sys.z80().mem.dump(offset, size) << std::endl;
					break;
				}
				case 'n':
					debug = false;
					break;
				case 'i':
					sys.z80().int_nmi = true;
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
						"\ti = NMI\n"
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

	} while (running && sys.clock());

	return EXIT_SUCCESS;
}
