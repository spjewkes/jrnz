/**
 * @brief Header defining the core of the z80 implementation.
 */

#ifndef __Z80_HPP__
#define __Z80_HPP__

#include <map>
#include "memory.hpp"
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
			map_inst.emplace(0x47, Instruction{std::string("ld b, a"), 1, 4, inst_ld, Operand::B, Operand::DE});
			map_inst.emplace(0xaf, Instruction{std::string("xor a"), 1, 4, inst_xor, Operand::A, Operand::UNUSED});
			map_inst.emplace(0xc3, Instruction{std::string("jp **"), 3, 10, inst_jp_nn, Operand::NN, Operand::UNUSED});
			map_inst.emplace(0xf3, Instruction{std::string("di"), 1, 4, inst_di, Operand::UNUSED, Operand::UNUSED});
		}

	unsigned short i = { 0 };
	Register16 pc;
	Register16 sp;
	Register16 ix;
	Register16 iy;
	
	Register16 af;
	Register16 hl;
	Register16 bc;
	Register16 de;
	bool int_on = { false };

	Memory mem;

	bool clock()
		{
			bool found = false;

			const unsigned short curr_pc = pc.get();
			const unsigned char v = mem.read(curr_pc);
			auto search = map_inst.find(v);
			if(search != map_inst.end())
			{
				const Instruction &inst = search->second;
				mem.dump(curr_pc, inst.size);
				std::cout << inst.name << std::endl;
				pc.set(curr_pc + inst.size);
				found = inst.func(*this, curr_pc, inst.dst, inst.src);
			}

			if(!found)
			{
				std::cout << "Unknown byte:" << std::endl;
				mem.dump(curr_pc, 1);
			}
			return found;
		}

private:
	std::map<unsigned char, const Instruction> map_inst;
};

#endif // __Z80_HPP__

