/**
 * @brief Class that manage entire system.
 */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <cstdlib>

#include "z80.hpp"
#include "bus.hpp"

/**
 * Defines the system class.
 */
class System
{
public:
	System(Z80 &_z80, Bus &_bus) : _z80(_z80), _bus(_bus) {}
	virtual ~System() {}

	bool clock(bool no_cycles);

	Z80& z80() { return _z80; }
	Bus& bus() { return _bus; }

private:
	Z80 &_z80;
	Bus &_bus;
};

#endif // __SYSTEM_HPP__
