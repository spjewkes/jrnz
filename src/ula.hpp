/**
 * @brief Header defining the ULA implementation.
 */

#ifndef __ULA_HPP__
#define __ULA_HPP__

#include "z80.hpp"
#include "bus.hpp"

/**
 * @brief Class describing the ULA.
 */
class ULA
{
public:
	ULA(Z80 &_z80, Bus &_bus) : _z80(_z80), _bus(_bus) {}
	virtual ~ULA() {}

	void clock();

private:
	Z80 &_z80;
	Bus &_bus;
};

#endif // __ULA_HPP__
