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
			map_inst.emplace(0x00, Instruction{std::string("nop"), 1, 4, inst_nop, Operand::UNUSED, Operand::UNUSED});
			map_inst.emplace(0x11, Instruction{std::string("ld de,**"), 3, 10, inst_ld, Operand::DE, Operand::NN});
			map_inst.emplace(0x36, Instruction{std::string("ld (hl),*"), 2, 10, inst_ld, Operand::indHL, Operand::N});
			map_inst.emplace(0x3e, Instruction{std::string("ld a,*"), 2, 7, inst_ld, Operand::A, Operand::N});
			map_inst.emplace(0x47, Instruction{std::string("ld b,a"), 1, 4, inst_ld, Operand::B, Operand::A});
			map_inst.emplace(0x62, Instruction{std::string("ld h,d"), 1, 4, inst_ld, Operand::H, Operand::D});
			map_inst.emplace(0x6b, Instruction{std::string("ld l,e"), 1, 4, inst_ld, Operand::L, Operand::E});
			map_inst.emplace(0xaf, Instruction{std::string("xor a"), 1, 4, inst_xor, Operand::A, Operand::A});
			map_inst.emplace(0xc3, Instruction{std::string("jp **"), 3, 10, inst_jp, Operand::PC, Operand::NN});
			map_inst.emplace(0xd3, Instruction{std::string("out (*),a"), 2, 11, inst_out, Operand::PORT, Operand::A});
			map_inst.emplace(0xf3, Instruction{std::string("di"), 1, 4, inst_di, Operand::UNUSED, Operand::UNUSED});

			map_inst.emplace(0xed47, Instruction{std::string("ED ld i,a"), 2, 9, inst_ld, Operand::I, Operand::A});
		}

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

	Register16 ir;

	bool clock()
		{
			bool found = false;

			const auto opcode = get_opcode();
			auto search = map_inst.find(opcode);
			if(search != map_inst.end())
			{
				const Instruction &inst = search->second;
				std::cout << std::left << std::setw(20) << mem.dump(curr_opcode_pc, inst.size);
				std::cout << inst.name << std::endl;
				pc.set(curr_opcode_pc + inst.size);
				found = inst.func(*this, inst.dst, inst.src);
			}

			if(!found)
			{
				std::cerr << "Unhandled instruction error:" << std::endl;
				std::cerr << mem.dump(curr_opcode_pc, 4) << std::endl;
			}
			return found;
		}

private:
	unsigned int get_opcode()
		{
			curr_opcode_pc = pc.get();
			curr_operand_pc = curr_opcode_pc + 1;
			unsigned int opcode = mem.read(curr_opcode_pc);

			// Handled extended instructions
			switch(opcode)
			{
			case 0xed:
			case 0xcd:
			case 0xdd:
			case 0xfd:
			{
				opcode = (opcode << 8) | mem.read(curr_operand_pc);
				curr_operand_pc++;

				// Handle IX and IY bit instructions
				switch(opcode)
				{
				case 0xddcb:
				case 0xfdcb:
					opcode = (opcode << 8) | mem.read(curr_opcode_pc);
					curr_operand_pc++;
				}
			}
			}
			
			return opcode;
		}
	std::map<unsigned int, const Instruction> map_inst;
};

#endif // __Z80_HPP__

