/**
 * @brief Implementation of system class.
 */

#include <iostream>

#include "system.hpp"

void System::set_break(bool enable, uint16_t _break_pc)
{
	do_break = enable;
	break_pc = _break_pc;
}

bool System::break_ready()
{
	if (do_break && _z80.pc.get() == break_pc)
	{
		std::cout << "Enabled break at 0x" << std::hex << break_pc << std::dec << std::endl;
		break_enabled = true;
		return true;
	}

	return false;
}

bool System::clock()
{
	bool running = true;

	if (!break_step && break_ready())
	{
		bool debug = true;
		char ch;

		while (debug)
		{
			std::cout << "Executing: " << _z80.dump_instr_at_pc(_z80.pc.get()).str() << std::endl;
			std::cin >> ch;

			switch (ch)
			{
			case 'c':
				do_break = false;
				debug = false;
				break_enabled = false;
				break;
			case 's':
				std::cin >> break_step;
				debug = false;
				break;
			case 'r':
				_z80.dump();
				break;
			case 't':
				_z80.dump_sp();
				break;
			case 'd':
			{
				std::string str_offset;
				std::string str_size;
				std::cin >> str_offset >> str_size;
				size_t offset = strtoul(str_offset.c_str(), NULL, 0);
				size_t size = strtoul(str_size.c_str(), NULL, 0);
				std::cout << _z80.mem.dump(offset, size) << std::endl;
				break;
			}
			case 'n':
				debug = false;
				break;
			case 'i':
				_z80.int_nmi = true;
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

	if (break_step)
	{
		break_step--;
	}

	return running && _z80.clock(do_break);
}
