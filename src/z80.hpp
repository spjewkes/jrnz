/**
 * @brief Header defining the core of the z80 implementation.
 */

#ifndef __Z80_HPP__
#define __Z80_HPP__

#include <cstdint>
#include <map>
#include "memory.hpp"
#include "ports.hpp"
#include "register.hpp"
#include "instructions.hpp"

/**
 * @brief Class describing a Z80 state.
 */
class Z80
{
public:
	Z80(size_t ram_size, std::string &rom_file);

	uint16_t curr_opcode_pc = { 0 }; // Stores the PC of the opcode under execution
	uint16_t curr_operand_pc = { 0 }; // Stores the PC of the expected first operand (if there are any) of the opcode under execution
	uint16_t top_of_stack = { 0 }; // Stores the expected top of the stack (for aiding debugging)
	Register16 pc;
	Register16 sp;
	Register16 ix;
	Register16 iy;

	RegisterAF af;
	Register16 hl;
	Register16 bc;
	Register16 de;
	bool int_enabled = { false };
	uint8_t int_mode = { 0 };
	bool int_nmi = { false };

	Memory mem;
	Ports ports;

	Register16 ir;

	bool clock();
	std::stringstream dump_instr_at_pc(uint16_t pc);
	void dump();
	void dump_sp();
	void reset();

private:
	uint32_t get_opcode(uint16_t pc);

	std::map<uint32_t, Instruction> map_inst;
	std::map<uint32_t, std::string> map_rom;
};

#endif // __Z80_HPP__

