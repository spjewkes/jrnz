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
			map_inst.emplace(0x00, Instruction{InstType::NOP,  "nop",        1,  4});
			map_inst.emplace(0x01, Instruction{InstType::LD,   "ld bc,**",   3, 10, Operand::BC, Operand::NN});
			map_inst.emplace(0x04, Instruction{InstType::INC,  "inc b",      1,  4, Operand::B, Operand::ONE});
			map_inst.emplace(0x0e, Instruction{InstType::LD,   "ld c,*",     2,  7, Operand::C, Operand::N});
			map_inst.emplace(0x10, Instruction{InstType::DJNZ, "djnz *",     2, 13, 8, Conditional::NZ, Operand::PC, Operand::N});
			map_inst.emplace(0x11, Instruction{InstType::LD,   "ld de,**",   3, 10, Operand::DE, Operand::NN});
			map_inst.emplace(0x19, Instruction{InstType::ADD,  "add hl,de",  1,  1, Operand::HL, Operand::DE});
			map_inst.emplace(0x20, Instruction{InstType::JR,   "jr nz,*",    2, 12, 7, Conditional::NZ, Operand::PC, Operand::N});
			map_inst.emplace(0x21, Instruction{InstType::LD,   "ld hl,**",   3, 10, Operand::HL, Operand::NN});
			map_inst.emplace(0x22, Instruction{InstType::LD,   "ld (**),hl", 3, 16, Operand::indNN, Operand::HL});
			map_inst.emplace(0x23, Instruction{InstType::INC,  "inc hl",     1,  6, Operand::HL, Operand::ONE});
			map_inst.emplace(0x28, Instruction{InstType::JR,   "jr z,*",     2, 12, 7, Conditional::Z, Operand::PC, Operand::N});
			map_inst.emplace(0x2a, Instruction{InstType::LD,   "ld hl,(**)", 3, 16, Operand::HL, Operand::indNN});
			map_inst.emplace(0x2b, Instruction{InstType::DEC,  "dec hl",     1,  6, Operand::HL, Operand::ONE});
			map_inst.emplace(0x30, Instruction{InstType::JR,   "jr nc,*",    2, 12, 7, Conditional::NC, Operand::PC, Operand::N});
			map_inst.emplace(0x32, Instruction{InstType::LD,   "ld (**),a",  3, 12, Operand::indN, Operand::A});
			map_inst.emplace(0x35, Instruction{InstType::DEC,  "dec (hl)",   1, 11, Operand::indHL, Operand::ONE});
			map_inst.emplace(0x36, Instruction{InstType::LD,   "ld (hl),*",  2, 10, Operand::indHL, Operand::N});
			map_inst.emplace(0x3e, Instruction{InstType::LD,   "ld a,*",     2,  7, Operand::A, Operand::N});
			map_inst.emplace(0x47, Instruction{InstType::LD,   "ld b,a",     1,  4, Operand::B, Operand::A});
			map_inst.emplace(0x62, Instruction{InstType::LD,   "ld h,d",     1,  4, Operand::H, Operand::D});
			map_inst.emplace(0x6b, Instruction{InstType::LD,   "ld l,e",     1,  4, Operand::L, Operand::E});
			map_inst.emplace(0x77, Instruction{InstType::LD,   "ld (hl),a",  1,  7, Operand::indHL, Operand::A});
			map_inst.emplace(0x78, Instruction{InstType::LD,   "ld a,b",     1,  4, Operand::A, Operand::B});
			map_inst.emplace(0x90, Instruction{InstType::SUB,  "sub b",      1,  4, Operand::A, Operand::B});
			map_inst.emplace(0x91, Instruction{InstType::SUB,  "sub c",      1,  4, Operand::A, Operand::C});
			map_inst.emplace(0x92, Instruction{InstType::SUB,  "sub d",      1,  4, Operand::A, Operand::D});
			map_inst.emplace(0x93, Instruction{InstType::SUB,  "sub e",      1,  4, Operand::A, Operand::E});
			map_inst.emplace(0x94, Instruction{InstType::SUB,  "sub h",      1,  4, Operand::A, Operand::H});
			map_inst.emplace(0x95, Instruction{InstType::SUB,  "sub l",      1,  4, Operand::A, Operand::L});
			map_inst.emplace(0x96, Instruction{InstType::SUB,  "sub (hl)",   1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0x97, Instruction{InstType::SUB,  "sub a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xa7, Instruction{InstType::AND,  "and a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xaf, Instruction{InstType::XOR,  "xor a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xbc, Instruction{InstType::CP,   "cp h",       1,  4, Operand::A, Operand::H});
			map_inst.emplace(0xc3, Instruction{InstType::JP,   "jp **",      3, 10, Conditional::ALWAYS, Operand::PC, Operand::NN});
			map_inst.emplace(0xc5, Instruction{InstType::PUSH, "push bc",    1, 11, Operand::UNUSED, Operand::BC});
			map_inst.emplace(0xcd, Instruction{InstType::CALL, "call **",    3, 17, Conditional::ALWAYS, Operand::PC, Operand::NN});
			map_inst.emplace(0xd3, Instruction{InstType::OUT,  "out (*),a",  2, 11, Operand::PORT, Operand::A});
			map_inst.emplace(0xd5, Instruction{InstType::PUSH, "push de",    1, 11, Operand::UNUSED, Operand::DE});
			map_inst.emplace(0xd6, Instruction{InstType::SUB,  "sub *",      2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xd9, Instruction{InstType::EX,   "exx",        1,  4});
			map_inst.emplace(0xe5, Instruction{InstType::PUSH, "push hl",    1, 11, Operand::UNUSED, Operand::HL});
			map_inst.emplace(0xeb, Instruction{InstType::EX,   "ex de,hl",   1,  4, Operand::DE, Operand::HL});
			map_inst.emplace(0xf3, Instruction{InstType::DI,   "di",         1,  4});
			map_inst.emplace(0xf5, Instruction{InstType::PUSH, "push af",    1, 11, Operand::UNUSED, Operand::AF});
			map_inst.emplace(0xf9, Instruction{InstType::LD,   "ld sp,hl",   1,  6, Operand::SP, Operand::HL});
			map_inst.emplace(0xfb, Instruction{InstType::EI,   "ei",         1,  4});

			map_inst.emplace(0xdd96, Instruction{InstType::SUB,  "sub (ix+*)", 3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xdde5, Instruction{InstType::PUSH, "push ix",    2, 15, Operand::UNUSED, Operand::IX});

			map_inst.emplace(0xddcb46, Instruction{InstType::BIT, "bit 0,(ix+*)", 4, 20, Operand::indIXN, Operand::ZERO});
			map_inst.emplace(0xddcb4e, Instruction{InstType::BIT, "bit 1,(ix+*)", 4, 20, Operand::indIXN, Operand::ONE});
			map_inst.emplace(0xddcb56, Instruction{InstType::BIT, "bit 2,(ix+*)", 4, 20, Operand::indIXN, Operand::TWO});
			map_inst.emplace(0xddcb5e, Instruction{InstType::BIT, "bit 3,(ix+*)", 4, 20, Operand::indIXN, Operand::THREE});
			map_inst.emplace(0xddcb66, Instruction{InstType::BIT, "bit 4,(ix+*)", 4, 20, Operand::indIXN, Operand::FOUR});
			map_inst.emplace(0xddcb6e, Instruction{InstType::BIT, "bit 5,(ix+*)", 4, 20, Operand::indIXN, Operand::FIVE});
			map_inst.emplace(0xddcb76, Instruction{InstType::BIT, "bit 6,(ix+*)", 4, 20, Operand::indIXN, Operand::SIX});
			map_inst.emplace(0xddcb7e, Instruction{InstType::BIT, "bit 7,(ix+*)", 4, 20, Operand::indIXN, Operand::SEVEN});
			map_inst.emplace(0xddcb86, Instruction{InstType::RES, "res 0,(ix+*)", 4, 23, Operand::indIXN, Operand::ZERO});
			map_inst.emplace(0xddcb8e, Instruction{InstType::RES, "res 1,(ix+*)", 4, 23, Operand::indIXN, Operand::ONE});
			map_inst.emplace(0xddcb96, Instruction{InstType::RES, "res 2,(ix+*)", 4, 23, Operand::indIXN, Operand::TWO});
			map_inst.emplace(0xddcb9e, Instruction{InstType::RES, "res 3,(ix+*)", 4, 23, Operand::indIXN, Operand::THREE});
			map_inst.emplace(0xddcba6, Instruction{InstType::RES, "res 4,(ix+*)", 4, 23, Operand::indIXN, Operand::FOUR});
			map_inst.emplace(0xddcbae, Instruction{InstType::RES, "res 5,(ix+*)", 4, 23, Operand::indIXN, Operand::FIVE});
			map_inst.emplace(0xddcbb6, Instruction{InstType::RES, "res 6,(ix+*)", 4, 23, Operand::indIXN, Operand::SIX});
			map_inst.emplace(0xddcbbe, Instruction{InstType::RES, "res 7,(ix+*)", 4, 23, Operand::indIXN, Operand::SEVEN});
			map_inst.emplace(0xddcbc6, Instruction{InstType::SET, "set 0,(ix+*)", 4, 23, Operand::indIXN, Operand::ZERO});
			map_inst.emplace(0xddcbce, Instruction{InstType::SET, "set 1,(ix+*)", 4, 23, Operand::indIXN, Operand::ONE});
			map_inst.emplace(0xddcbd6, Instruction{InstType::SET, "set 2,(ix+*)", 4, 23, Operand::indIXN, Operand::TWO});
			map_inst.emplace(0xddcbde, Instruction{InstType::SET, "set 3,(ix+*)", 4, 23, Operand::indIXN, Operand::THREE});
			map_inst.emplace(0xddcbe6, Instruction{InstType::SET, "set 4,(ix+*)", 4, 23, Operand::indIXN, Operand::FOUR});
			map_inst.emplace(0xddcbee, Instruction{InstType::SET, "set 5,(ix+*)", 4, 23, Operand::indIXN, Operand::FIVE});
			map_inst.emplace(0xddcbf6, Instruction{InstType::SET, "set 6,(ix+*)", 4, 23, Operand::indIXN, Operand::SIX});
			map_inst.emplace(0xddcbfe, Instruction{InstType::SET, "set 7,(ix+*)", 4, 23, Operand::indIXN, Operand::SEVEN});

			map_inst.emplace(0xed43, Instruction{InstType::LD,   "ld (**),bc", 4, 20, Operand::indNN, Operand::BC});
			map_inst.emplace(0xed47, Instruction{InstType::LD,   "ld i,a",     2,  9, Operand::I, Operand::A});
			map_inst.emplace(0xed52, Instruction{InstType::SBC,  "sbc hl,de",  2, 15, Operand::HL, Operand::DE});
			map_inst.emplace(0xed53, Instruction{InstType::LD,   "ld (**),de", 4, 20, Operand::indNN, Operand::DE});
			map_inst.emplace(0xed56, Instruction{InstType::IM,   "im 1",       2,  8, Operand::IM, Operand::ONE});
			map_inst.emplace(0xedb0, Instruction{InstType::LDIR, "ldir",       2, 21, 16, Operand::indDE, Operand::indHL});
			map_inst.emplace(0xedb8, Instruction{InstType::LDDR, "lddr",       2, 21, 16, Operand::indDE, Operand::indHL});

			map_inst.emplace(0xfd21, Instruction{InstType::LD,   "ld iy,**",     4, 14, Operand::IY, Operand::NN});
			map_inst.emplace(0xfd35, Instruction{InstType::DEC,  "dec (iy+*)",   3, 23, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfd75, Instruction{InstType::LD,   "ld (iy+*),l",  3, 19, Operand::indIYN, Operand::L});
			map_inst.emplace(0xfd86, Instruction{InstType::ADD,  "add a,(iy+*)", 3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfd96, Instruction{InstType::SUB,  "sub (iy+*)",   3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfde5, Instruction{InstType::PUSH, "push iy",      2, 15, Operand::UNUSED, Operand::IY});

			map_inst.emplace(0xfdcb46, Instruction{InstType::BIT, "bit 0,(iy+*)", 4, 20, Operand::indIYN, Operand::ZERO});
			map_inst.emplace(0xfdcb4e, Instruction{InstType::BIT, "bit 1,(iy+*)", 4, 20, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfdcb56, Instruction{InstType::BIT, "bit 2,(iy+*)", 4, 20, Operand::indIYN, Operand::TWO});
			map_inst.emplace(0xfdcb5e, Instruction{InstType::BIT, "bit 3,(iy+*)", 4, 20, Operand::indIYN, Operand::THREE});
			map_inst.emplace(0xfdcb66, Instruction{InstType::BIT, "bit 4,(iy+*)", 4, 20, Operand::indIYN, Operand::FOUR});
			map_inst.emplace(0xfdcb6e, Instruction{InstType::BIT, "bit 5,(iy+*)", 4, 20, Operand::indIYN, Operand::FIVE});
			map_inst.emplace(0xfdcb76, Instruction{InstType::BIT, "bit 6,(iy+*)", 4, 20, Operand::indIYN, Operand::SIX});
			map_inst.emplace(0xfdcb7e, Instruction{InstType::BIT, "bit 7,(iy+*)", 4, 20, Operand::indIYN, Operand::SEVEN});
			map_inst.emplace(0xfdcb86, Instruction{InstType::RES, "res 0,(iy+*)", 4, 23, Operand::indIYN, Operand::ZERO});
			map_inst.emplace(0xfdcb8e, Instruction{InstType::RES, "res 1,(iy+*)", 4, 23, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfdcb96, Instruction{InstType::RES, "res 2,(iy+*)", 4, 23, Operand::indIYN, Operand::TWO});
			map_inst.emplace(0xfdcb9e, Instruction{InstType::RES, "res 3,(iy+*)", 4, 23, Operand::indIYN, Operand::THREE});
			map_inst.emplace(0xfdcba6, Instruction{InstType::RES, "res 4,(iy+*)", 4, 23, Operand::indIYN, Operand::FOUR});
			map_inst.emplace(0xfdcbae, Instruction{InstType::RES, "res 5,(iy+*)", 4, 23, Operand::indIYN, Operand::FIVE});
			map_inst.emplace(0xfdcbb6, Instruction{InstType::RES, "res 6,(iy+*)", 4, 23, Operand::indIYN, Operand::SIX});
			map_inst.emplace(0xfdcbbe, Instruction{InstType::RES, "res 7,(iy+*)", 4, 23, Operand::indIYN, Operand::SEVEN});
			map_inst.emplace(0xfdcbc6, Instruction{InstType::SET, "set 0,(iy+*)", 4, 23, Operand::indIYN, Operand::ZERO});
			map_inst.emplace(0xfdcbce, Instruction{InstType::SET, "set 1,(iy+*)", 4, 23, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfdcbd6, Instruction{InstType::SET, "set 2,(iy+*)", 4, 23, Operand::indIYN, Operand::TWO});
			map_inst.emplace(0xfdcbde, Instruction{InstType::SET, "set 3,(iy+*)", 4, 23, Operand::indIYN, Operand::THREE});
			map_inst.emplace(0xfdcbe6, Instruction{InstType::SET, "set 4,(iy+*)", 4, 23, Operand::indIYN, Operand::FOUR});
			map_inst.emplace(0xfdcbee, Instruction{InstType::SET, "set 5,(iy+*)", 4, 23, Operand::indIYN, Operand::FIVE});
			map_inst.emplace(0xfdcbf6, Instruction{InstType::SET, "set 6,(iy+*)", 4, 23, Operand::indIYN, Operand::SIX});
			map_inst.emplace(0xfdcbfe, Instruction{InstType::SET, "set 7,(iy+*)", 4, 23, Operand::indIYN, Operand::SEVEN});
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
	unsigned char int_mode = { 0 };

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
				Instruction &inst = search->second;
				std::cout << std::left << std::setw(20) << mem.dump(curr_opcode_pc, inst.size);
				std::cout << inst.name << std::endl;
				pc.set(curr_opcode_pc + inst.size);
				found = inst.execute(*this);
			}

			if(!found)
			{
				std::cerr << "Unhandled instruction error: 0x" << std::hex << opcode << std::dec << std::endl;
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
			case 0xcb:
			case 0xdd:
			case 0xfd:
			{
				opcode = (opcode << 8) | mem.read(curr_operand_pc);
				curr_operand_pc++;

				// Handle IX and IY bit instructions, the opcode comes after
				// the displacement byte:
				// 0xddcb <displacement byte> <opcode>
				// oxfdcb <displacement byte> <opcode>
				// Make sure the operand is not in the returned opcode
				switch(opcode)
				{
				case 0xddcb:
				case 0xfdcb:
					opcode = (opcode << 8) | mem.read(curr_operand_pc+1);
					// Don't increment the operand any further
				}
			}
			}
			
			return opcode;
		}
	std::map<unsigned int, Instruction> map_inst;
};

#endif // __Z80_HPP__

