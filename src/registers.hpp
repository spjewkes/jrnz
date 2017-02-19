/**
 * @brief Header managing registers and thier state.
 */

#ifndef __REGISTERS_HPP__
#define __REGISTERS_HPP__

#include <map>
#include "memory.hpp"
#include "instructions.hpp"

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
	defined(__BIG_ENDIAN__) || \
	defined(__ARMEB__) || \
	defined(__THUMBEB__) || \
	defined(__AARCH64EB__) || \
	defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
// It's a big-endian target architecture
#define HI_REG8 (1)
#define LO_REG8 (0)
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
	defined(__LITTLE_ENDIAN__) || \
	defined(__ARMEL__) || \
	defined(__THUMBEL__) || \
	defined(__AARCH64EL__) || \
	defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
// It's a little-endian target architecture
#define HI_REG8 (0)
#define LO_REG8 (1)
#else
#error "I don't know what architecture this is!"
#endif

/**
 * @brief Class describing a 16-bit register.
 */
class Register16
{
public:
	void hi(unsigned char v) { reg = ((v << 8) & 0xff00) | (reg & 0xff); }
	unsigned char hi() const { return static_cast<unsigned char>((reg>>8) & 0xff); }
	void lo(unsigned char v) { reg = (reg & 0xff00) | (v & 0xff); }
	unsigned char lo() const { return static_cast<unsigned char>(reg & 0xff); }
	void set(unsigned short v) { reg = v; }
	unsigned short get() const { return reg; }
	void swap() { std::swap(reg, alt_reg); }

	StorageElement element() { return StorageElement(&c_reg[0], 2); }
	StorageElement element_hi() { return StorageElement(&c_reg[HI_REG8], 1); }
	StorageElement element_lo() { return StorageElement(&c_reg[LO_REG8], 1); }

private:
	union
	{
		unsigned short reg = 0;
		unsigned char c_reg[2];
	};
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
