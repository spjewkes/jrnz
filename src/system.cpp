/**
 * @brief Implementation of system class.
 */

#include <iostream>

#include "system.hpp"

bool System::clock(bool no_cycles)
{
	return _z80.clock(no_cycles);
}
