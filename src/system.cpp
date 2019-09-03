/**
 * @brief Implementation of system class.
 */

#include <iostream>

#include "system.hpp"

void System::set_break(bool enable, uint16_t _break_pc)
{
	break_enabled = enable;
	break_pc = _break_pc;
}

bool System::break_ready()
{
	if (break_enabled && _z80.pc.get() == break_pc)
	{
		std::cout << "Enabled break at 0x" << std::hex << break_pc << std::dec << std::endl;
		return true;
	}

	return false;
}

bool System::clock()
{
	return _z80.clock();
}
