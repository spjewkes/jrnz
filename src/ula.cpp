/**
 * Brief Implementation of the ULA class.
 */

#include "ula.hpp"

void ULA::clock()
{
	UNUSED(_z80);
	UNUSED(_bus);

	switch(counter)
	{
	case 0:
		// Trigger interupt on Z80
		_z80.interrupt = true;
		break;

	case 32:
		// Turn off interrupt
		_z80.interrupt = false;
		break;

	case 70000:
		//! TODO - a bit of a hack, but every 50th of a second (running at 3.5 Mhz)
	    //! reset the counter to start everything again.
		counter = UINT64_MAX; // will wrap on increment
	}

	counter++;
}
