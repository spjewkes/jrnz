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
	Z80(unsigned int ram_size, std::string &rom_file) : mem(ram_size, rom_file)
		{
			map_inst.emplace(0x11, Instruction{std::string("ld de,**"), 3, 10, inst_ld, Operand::DE, Operand::NN});
			map_inst.emplace(0x3e, Instruction{std::string("ld a,*"), 2, 7, inst_ld, Operand::A, Operand::N});
			map_inst.emplace(0x47, Instruction{std::string("ld b,a"), 1, 4, inst_ld, Operand::B, Operand::A});
			map_inst.emplace(0xaf, Instruction{std::string("xor a"), 1, 4, inst_xor, Operand::A, Operand::A});
			map_inst.emplace(0xc3, Instruction{std::string("jp **"), 3, 10, inst_jp, Operand::PC, Operand::NN});
			map_inst.emplace(0xd3, Instruction{std::string("out (*),a"), 2, 11, inst_out, Operand::PORT, Operand::A});
			map_inst.emplace(0xf3, Instruction{std::string("di"), 1, 4, inst_di, Operand::UNUSED, Operand::UNUSED});
		}

	unsigned short i = { 0 };
	unsigned short curr_opcode_pc = { 0 }; // Stores the PC of the opcode under execution
	unsigned short curr_operand_pc = { 0 }; // Stores the PC of the expected first operand (if there are any) of the opcode under execution
	Register16 pc;
	Register16 sp;
	Register16 ix;
	Register16 iy;
	
	RegisterAF af;
	Register16 hl;
	Register16 bc;
	Register16 de;
	bool int_on = { false };

	Memory mem;
	Ports ports;

	bool clock()
		{
			bool found = false;

			curr_opcode_pc = pc.get();
			const unsigned char v = mem.read(curr_opcode_pc);
			auto search = map_inst.find(v);
			if(search != map_inst.end())
			{
				const Instruction &inst = search->second;
				mem.dump(curr_opcode_pc, inst.size);
				std::cout << inst.name << std::endl;
				curr_operand_pc = curr_opcode_pc + 1;
				pc.set(curr_opcode_pc + inst.size);
				found = inst.func(*this, inst.dst, inst.src);
			}

			if(!found)
			{
				std::cout << "Unknown data:" << std::endl;
				mem.dump(curr_opcode_pc, 4);
			}
			return found;
		}

private:
	std::map<unsigned char, const Instruction> map_inst;
};

#endif // __Z80_HPP__

