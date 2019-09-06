/**
 * @brief Implementation of debugger class.
 */

#include "debugger.hpp"
#include "decoder.hpp"

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
			std::cout << "Executing: " << dump_instr_at_addr(_z80.pc.get()).str() << std::endl;
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
				dump();
				break;
			case 't':
				dump_sp();
				break;
			case 'd':
			{
				std::string str_offset;
				std::string str_size;
				std::cin >> str_offset >> str_size;
				size_t offset = strtoul(str_offset.c_str(), NULL, 0);
				size_t size = strtoul(str_size.c_str(), NULL, 0);
				std::cout << dump_mem_at_addr(offset, size).str() << std::endl;
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
	else
	{
		// Only dump current instruction we're not clocking the current instruction
		if (debug_out && !_z80.cycles_left)
		{
			std::cout << dump_instr_at_addr(_z80.pc.get()).str() << std::endl;
		}
	}

	if (break_step)
	{
		break_step--;
	}

	return running;
}

std::stringstream Debugger::dump_instr_at_addr(uint16_t addr)
{
	std::stringstream str;

	const auto opcode = _bus.read_opcode_from_mem(addr);
	const Instruction &inst = decode_opcode(opcode);
	if (inst.inst != InstType::INV)
	{
		str << std::left << std::setw(20) << dump_mem_at_addr(addr, inst.size).str();
		str << std::setw(20) << inst.name;

		if (has_rom_label(addr))
		{
			str << "Routine: " << decode_rom_label(addr);
		}
	}
	else
	{
		str << dump_mem_at_addr(addr, 4).str() << " UNKNOWN INSTRUCTION: 0x" << std::hex << opcode << std::dec;
	}

	return str;
}

std::stringstream Debugger::dump_mem_at_addr(uint16_t addr, size_t size, size_t per_line)
{
	std::stringstream str;

	uint16_t curr_addr = addr;

	for (size_t pos = 0; pos < size; pos++)
	{
		if ((pos % per_line) == 0)
		{
			str << std::hex << "0x" << std::setw(4) << std::setfill('0') << curr_addr << ":";
		}

		str << std::hex << " " << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(_bus.read_data(curr_addr));

		if ((pos % per_line) == (per_line - 1))
		{
			str << std::endl;
		}

		curr_addr++;
	}

	return str;
}

void Debugger::dump()
{
	std::cout << "AF: " << _z80.af << std::endl;
	std::cout << "PC: " << _z80.pc << std::endl;
	std::cout << "SP: " << _z80.sp << std::endl;
	std::cout << "BC: " << _z80.bc << std::endl;
	std::cout << "DE: " << _z80.de << std::endl;
	std::cout << "HL: " << _z80.hl << std::endl;
	std::cout << "IX: " << _z80.ix << std::endl;
	std::cout << "IY: " << _z80.iy << std::endl;
	std::cout << "IM: " << static_cast<uint32_t>(_z80.int_mode) << " ei: " << (_z80.int_enabled ? "on" : "off") << std::endl;
}

void Debugger::dump_sp()
{
	assert(_z80.sp.get() <= _z80.top_of_stack);
	std::cout << "Dumping stack at SP: " << _z80.sp << std::endl;
	std::cout << dump_mem_at_addr(_z80.sp.get(), _z80.top_of_stack - _z80.sp.get()).str() << std::endl;
	std::cout << "==== TOP OF THE STACK ====" << std::endl;
}
