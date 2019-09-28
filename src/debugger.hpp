/**
 * @brief Class that handles the debugger.
 */

#ifndef __DEBUGGER_HPP__
#define __DEBUGGER_HPP__

#include <cstdlib>

#include "z80.hpp"
#include "bus.hpp"

/**
 * Defines the debugger class.
 */
class Debugger
{
public:
	Debugger(Z80 &_z80, Bus &_bus) : _z80(_z80), _bus(_bus) {}
	virtual ~Debugger() {}

	void set_dout(bool enable) { debug_out = enable; }
	void set_break(bool enable, uint16_t _break_pc = 0x0000);
	bool break_ready();
	bool is_break_enabled() const { return break_enabled; }

	bool clock();

	std::stringstream dump_instr_at_addr(uint16_t addr);
	std::stringstream dump_mem_at_addr(uint16_t addr, size_t size, size_t per_line = 16);

	void dump();
	void dump_sp();

public:
	Z80 &_z80;
	Bus &_bus;

	bool debug_out = { false };
	size_t break_step = { 0ull };
	bool break_enabled = { false };
	bool break_at_pc = { false };
	uint16_t break_pc = { 0x0000 };
	uint16_t break_pc_tmp = { 0x0000 };
};

#endif // __DEBUGGER_HPP__
