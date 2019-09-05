/**
 * @brief Implementation of system class.
 */

#include <iostream>

#include "system.hpp"

bool System::clock(bool no_cycles)
{
	_ula.clock();
	return _z80.clock(no_cycles);
}
