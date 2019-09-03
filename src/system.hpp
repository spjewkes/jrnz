/**
 * @brief Class that manage entire system.
 */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include "z80.hpp"
#include "memory.hpp"

/**
 * Defines the system class.
 */
class System
{
public:
	System(Z80 &_z80, Memory &_memory) : _z80(_z80), _memory(_memory) {}
	virtual ~System() {}

	bool clock();

	Z80& z80() { return _z80; }
	Memory& memory() { return _memory; }

private:
	Z80 _z80;
	Memory _memory;
};

#endif // __SYSTEM_HPP__
