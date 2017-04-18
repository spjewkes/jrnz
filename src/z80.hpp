/**
 * @brief Header defining the core of the z80 implementation.
 */

#ifndef __Z80_HPP__
#define __Z80_HPP__

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
	Z80(unsigned int ram_size, std::string &rom_file);

	unsigned short curr_opcode_pc = { 0 }; // Stores the PC of the opcode under execution
	unsigned short curr_operand_pc = { 0 }; // Stores the PC of the expected first operand (if there are any) of the opcode under execution
	unsigned short top_of_stack = { 0 }; // Stores the expected top of the stack (for aiding debugging)
	Register16 pc;
	Register16 sp;
	Register16 ix;
	Register16 iy;

	RegisterAF af;
	Register16 hl;
	Register16 bc;
	Register16 de;
	bool int_on = { false };
	unsigned char int_mode = { 0 };

	Memory mem;
	Ports ports;

	Register16 ir;

	bool clock();
	std::stringstream dump_instr_at_pc(unsigned short pc);
	void dump();
	void dump_sp();
	void reset();

private:
	unsigned int get_opcode(unsigned short pc);

	std::map<unsigned int, Instruction> map_inst;
	std::map<unsigned int, std::string> map_rom;
};

#endif // __Z80_HPP__

