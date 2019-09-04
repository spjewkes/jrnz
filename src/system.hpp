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
	System(Z80 &_z80, Bus &_memory) : _z80(_z80), _memory(_memory) {}
	virtual ~System() {}

	bool clock(bool no_cycles);

	Z80& z80() { return _z80; }
	Bus& memory() { return _memory; }

private:
	Z80 &_z80;
	Bus &_memory;
};

#endif // __SYSTEM_HPP__
