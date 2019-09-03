/**
 * @brief Implementation of debugger class.
 */

#include "debugger.hpp"

void Debugger::set_break(bool enable, uint16_t _break_pc)
{
	break_at_pc = enable;
	break_pc = _break_pc;
}

bool Debugger::break_ready()
{
	if (break_at_pc && _z80.pc.get() == break_pc)
	{
		std::cout << "Enabled break at 0x" << std::hex << break_pc << std::dec << std::endl;
		break_enabled = true;
		break_at_pc = false;
		return true;
	}

	return false;
}

bool Debugger::clock()
{
	bool running = true;

	if ((break_enabled && !break_step) || break_ready())
	{
		bool paused = true;
		char ch;

		while (paused)
		{
			std::cout << "Executing: " << _z80.dump_instr_at_pc(_z80.pc.get()).str() << std::endl;
			std::cin >> ch;

			switch (ch)
			{
			case 'b':
				break_at_pc = true;
				std::cin >> std::hex >> break_pc;
				paused = false;
				break_enabled = false;
				break;
			case 'c':
				break_at_pc = false;
				paused = false;
				break_enabled = false;
				break;
			case 's':
				std::cin >> break_step;
				paused = false;
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
				paused = false;
				break;
			case 'i':
				_z80.int_nmi = true;
				break;
			case 'q':
				running = false;
				paused = false;
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

	return running;
}
