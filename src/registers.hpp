/**
 * @brief Header managing registers and thier state.
 */

#ifndef __REGISTERS_HPP__
#define __REGISTERS_HPP__

#include <map>
#include "memory.hpp"
#include "instructions.hpp"

/**
 * @brief Class describing an 8-bit register.
 */
class Register8
{
public:
	unsigned char get() const { return reg; }
	void set(unsigned char v) { reg = v; }

	void swap() { std::swap(reg, alt_reg); }

private:
	unsigned char reg = 0;
	unsigned char alt_reg = 0;
};

/**
 * @brief Class describing a 16-bit register.
 */
class Register16
{
public:
	void hi(unsigned char v) { rh.set(v); }
	unsigned char hi() const { return rh.get(); }
	void lo(unsigned char v) { rl.set(v); }
	unsigned char lo() const { return rl.get(); }
	void set(unsigned short v)
		{
			rl.set(static_cast<unsigned char>(v&0xff));
			rh.set(static_cast<unsigned char>((v>>8)&0xff));
		}
	unsigned short get() const
		{
			unsigned short v = rl.get();
			v |= rh.get() << 8;
			return v;
		}
	void swap() { rl.swap(); rh.swap(); }

private:
	Register8 rl;
	Register8 rh;
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
	Register16 pc;
	unsigned short sp = { 0 };
	unsigned short ix = { 0 };
	unsigned short iy = { 0 };
	
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

#endif // __REGISTERS_HPP__
