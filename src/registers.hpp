/**
 * @brief Header managing registers and thier state.
 */

#ifndef __REGISTERS_HPP__
#define __REGISTERS_HPP__

#include <map>
#include "memory.hpp"
#include "instructions.hpp"

/**
 * @brief Class describing a 16-bit register.
 */
class Register
{
public:
	void hi(unsigned char v)
		{
			reg &= 0x00ff;
			reg |= v << 8;
		}
	unsigned char hi() const { return static_cast<unsigned char>((reg >> 8) & 0xff); }
	void lo(unsigned char v)
		{
			reg &= 0xff00;
			reg |= v;
		}
	unsigned char lo() const { return static_cast<unsigned char>(reg & 0xff); }
	void set(unsigned short v) { reg = v; }
	unsigned short get() const { return reg; }
	void swap() { std::swap(reg, alt_reg); }

private:
	unsigned short reg = 0;
	unsigned short alt_reg = 0;
};

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
	Register pc;
	unsigned short sp = { 0 };
	unsigned short ix = { 0 };
	unsigned short iy = { 0 };
	
	Register af;
	Register hl;
	Register bc;
	Register de;
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

#endif // __REGISTERS_HPP__
