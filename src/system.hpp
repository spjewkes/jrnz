/**
 * @brief Class that manage entire system.
 */

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <cstdlib>

#include "z80.hpp"
#include "bus.hpp"
#include "ula.hpp"

/**
 * Defines the system class.
 */
class System
{
public:
	System(Z80 &_z80, ULA &_ula, Bus &_bus) : _z80(_z80), _ula(_ula), _bus(_bus) {}
	virtual ~System() {}

	bool clock(bool no_cycles);

	Z80& z80() { return _z80; }
	ULA& ula() { return _ula; }
	Bus& bus() { return _bus; }

private:
	Z80 &_z80;
	ULA &_ula;
	Bus &_bus;
};

#endif // __SYSTEM_HPP__
