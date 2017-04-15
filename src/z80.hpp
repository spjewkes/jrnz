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
			reset();

			map_inst.emplace(0x00, Instruction{InstType::NOP,  "nop",        1,  4});
			map_inst.emplace(0x01, Instruction{InstType::LD,   "ld bc,**",   3, 10, Operand::BC, Operand::NN});
			map_inst.emplace(0x02, Instruction{InstType::LD,   "ld (bc),a",  1,  7, Operand::indBC, Operand::A});
			map_inst.emplace(0x03, Instruction{InstType::INC,  "inc bc",     1,  6, Operand::BC, Operand::ONE});
			map_inst.emplace(0x04, Instruction{InstType::INC,  "inc b",      1,  4, Operand::B, Operand::ONE});
			map_inst.emplace(0x05, Instruction{InstType::DEC,  "dec b",      1,  4, Operand::B, Operand::ONE});
			map_inst.emplace(0x06, Instruction{InstType::LD,   "ld b,*",     2,  7, Operand::B, Operand::N});
			map_inst.emplace(0x08, Instruction{InstType::EX,   "ex af,af'",  1,  4, Operand::AF, Operand::UNUSED});
			map_inst.emplace(0x09, Instruction{InstType::ADD,  "add hl,bc",  1, 11, Operand::HL, Operand::BC});
			map_inst.emplace(0x0a, Instruction{InstType::LD,   "ld a,(bc)",  1,  7, Operand::A, Operand::indBC});
			map_inst.emplace(0x0b, Instruction{InstType::DEC,  "dec bc",     1,  6, Operand::BC, Operand::ONE});
			map_inst.emplace(0x0c, Instruction{InstType::INC,  "inc c",      1,  4, Operand::C, Operand::ONE});
			map_inst.emplace(0x0d, Instruction{InstType::DEC,  "dec c",      1,  4, Operand::C, Operand::ONE});
			map_inst.emplace(0x0e, Instruction{InstType::LD,   "ld c,*",     2,  7, Operand::C, Operand::N});
			map_inst.emplace(0x0f, Instruction{InstType::RRCA, "rrca",       1,  4, Operand::A});
			map_inst.emplace(0x10, Instruction{InstType::DJNZ, "djnz *",     2, 13, 8, Conditional::NZ, Operand::PC, Operand::N});
			map_inst.emplace(0x11, Instruction{InstType::LD,   "ld de,**",   3, 10, Operand::DE, Operand::NN});
			map_inst.emplace(0x12, Instruction{InstType::LD,   "ld (de),a",  1,  7, Operand::indDE, Operand::A});
			map_inst.emplace(0x13, Instruction{InstType::INC,  "inc de",     1,  6, Operand::DE, Operand::ONE});
			map_inst.emplace(0x14, Instruction{InstType::INC,  "inc d",      1,  4, Operand::D, Operand::ONE});
			map_inst.emplace(0x15, Instruction{InstType::DEC,  "dec d",      1,  4, Operand::D, Operand::ONE});
			map_inst.emplace(0x16, Instruction{InstType::LD,   "ld d,*",     2,  7, Operand::D, Operand::N});
			map_inst.emplace(0x18, Instruction{InstType::JR,   "jr *",       2, 18, Conditional::ALWAYS, Operand::PC, Operand::N});
			map_inst.emplace(0x19, Instruction{InstType::ADD,  "add hl,de",  1,  1, Operand::HL, Operand::DE});
			map_inst.emplace(0x1a, Instruction{InstType::LD,   "ld a,(de)",  1,  7, Operand::A, Operand::indDE});
			map_inst.emplace(0x1b, Instruction{InstType::DEC,  "dec de",     1,  6, Operand::DE, Operand::ONE});
			map_inst.emplace(0x1c, Instruction{InstType::INC,  "inc e",      1,  4, Operand::E, Operand::ONE});
			map_inst.emplace(0x1d, Instruction{InstType::DEC,  "dec e",      1,  4, Operand::E, Operand::ONE});
			map_inst.emplace(0x1e, Instruction{InstType::LD,   "ld e,*",     2,  7, Operand::E, Operand::N});
			map_inst.emplace(0x20, Instruction{InstType::JR,   "jr nz,*",    2, 12, 7, Conditional::NZ, Operand::PC, Operand::N});
			map_inst.emplace(0x21, Instruction{InstType::LD,   "ld hl,**",   3, 10, Operand::HL, Operand::NN});
			map_inst.emplace(0x22, Instruction{InstType::LD,   "ld (**),hl", 3, 16, Operand::indNN, Operand::HL});
			map_inst.emplace(0x23, Instruction{InstType::INC,  "inc hl",     1,  6, Operand::HL, Operand::ONE});
			map_inst.emplace(0x24, Instruction{InstType::INC,  "inc h",      1,  4, Operand::H, Operand::ONE});
			map_inst.emplace(0x25, Instruction{InstType::DEC,  "dec h",      1,  4, Operand::H, Operand::ONE});
			map_inst.emplace(0x26, Instruction{InstType::LD,   "ld h,*",     2,  7, Operand::H, Operand::N});
			map_inst.emplace(0x28, Instruction{InstType::JR,   "jr z,*",     2, 12, 7, Conditional::Z, Operand::PC, Operand::N});
			map_inst.emplace(0x29, Instruction{InstType::ADD,  "add hl,hl",  1, 11, Operand::HL, Operand::HL});
			map_inst.emplace(0x2a, Instruction{InstType::LD,   "ld hl,(**)", 3, 16, Operand::HL, Operand::indNN});
			map_inst.emplace(0x2b, Instruction{InstType::DEC,  "dec hl",     1,  6, Operand::HL, Operand::ONE});
			map_inst.emplace(0x2c, Instruction{InstType::INC,  "inc l",      1,  4, Operand::L, Operand::ONE});
			map_inst.emplace(0x2d, Instruction{InstType::DEC,  "dec l",      1,  4, Operand::L, Operand::ONE});
			map_inst.emplace(0x2e, Instruction{InstType::LD,   "ld l,*",     2,  7, Operand::L, Operand::N});
			map_inst.emplace(0x30, Instruction{InstType::JR,   "jr nc,*",    2, 12, 7, Conditional::NC, Operand::PC, Operand::N});
			map_inst.emplace(0x31, Instruction{InstType::LD,   "ld sp,**",   3, 10, Operand::SP, Operand::NN});
			map_inst.emplace(0x32, Instruction{InstType::LD,   "ld (**),a",  3, 12, Operand::indN, Operand::A});
			map_inst.emplace(0x33, Instruction{InstType::INC,  "inc sp",     1,  6, Operand::SP, Operand::ONE});
			map_inst.emplace(0x34, Instruction{InstType::INC,  "inc (hl)",   1, 11, Operand::indHL, Operand::ONE});
			map_inst.emplace(0x35, Instruction{InstType::DEC,  "dec (hl)",   1, 11, Operand::indHL, Operand::ONE});
			map_inst.emplace(0x36, Instruction{InstType::LD,   "ld (hl),*",  2, 10, Operand::indHL, Operand::N});
			map_inst.emplace(0x37, Instruction{InstType::SCF,  "scf",        1, 4});
			map_inst.emplace(0x38, Instruction{InstType::JR,   "jr c,*",     2, 12, 7, Conditional::C, Operand::PC, Operand::N});
			map_inst.emplace(0x39, Instruction{InstType::ADD,  "add hl,sp",  1, 11, Operand::HL, Operand::SP});
			map_inst.emplace(0x3a, Instruction{InstType::LD,   "ld a,(**)",  3, 13, Operand::A, Operand::indN});
			map_inst.emplace(0x3b, Instruction{InstType::DEC,  "dec sp",     1,  6, Operand::SP, Operand::ONE});
			map_inst.emplace(0x3c, Instruction{InstType::INC,  "inc a",      1,  4, Operand::A, Operand::ONE});
			map_inst.emplace(0x3d, Instruction{InstType::DEC,  "dec a",      1,  4, Operand::A, Operand::ONE});
			map_inst.emplace(0x3e, Instruction{InstType::LD,   "ld a,*",     2,  7, Operand::A, Operand::N});
			map_inst.emplace(0x3f, Instruction{InstType::CCF,  "ccf",        1,  4});
			map_inst.emplace(0x40, Instruction{InstType::LD,   "ld b,b",     1,  4, Operand::B, Operand::B});
			map_inst.emplace(0x41, Instruction{InstType::LD,   "ld b,c",     1,  4, Operand::B, Operand::C});
			map_inst.emplace(0x42, Instruction{InstType::LD,   "ld b,d",     1,  4, Operand::B, Operand::D});
			map_inst.emplace(0x43, Instruction{InstType::LD,   "ld b,e",     1,  4, Operand::B, Operand::E});
			map_inst.emplace(0x44, Instruction{InstType::LD,   "ld b,h",     1,  4, Operand::B, Operand::H});
			map_inst.emplace(0x45, Instruction{InstType::LD,   "ld b,l",     1,  4, Operand::B, Operand::L});
			map_inst.emplace(0x46, Instruction{InstType::LD,   "ld b,(hl)",  1,  7, Operand::B, Operand::indHL});
			map_inst.emplace(0x47, Instruction{InstType::LD,   "ld b,a",     1,  4, Operand::B, Operand::A});
			map_inst.emplace(0x48, Instruction{InstType::LD,   "ld c,b",     1,  4, Operand::C, Operand::B});
			map_inst.emplace(0x49, Instruction{InstType::LD,   "ld c,c",     1,  4, Operand::C, Operand::C});
			map_inst.emplace(0x4a, Instruction{InstType::LD,   "ld c,d",     1,  4, Operand::C, Operand::D});
			map_inst.emplace(0x4b, Instruction{InstType::LD,   "ld c,e",     1,  4, Operand::C, Operand::E});
			map_inst.emplace(0x4c, Instruction{InstType::LD,   "ld c,h",     1,  4, Operand::C, Operand::H});
			map_inst.emplace(0x4d, Instruction{InstType::LD,   "ld c,l",     1,  4, Operand::C, Operand::L});
			map_inst.emplace(0x4e, Instruction{InstType::LD,   "ld c,(hl)",  1,  7, Operand::C, Operand::indHL});
			map_inst.emplace(0x4f, Instruction{InstType::LD,   "ld c,a",     1,  4, Operand::C, Operand::A});
			map_inst.emplace(0x50, Instruction{InstType::LD,   "ld d,b",     1,  4, Operand::D, Operand::B});
			map_inst.emplace(0x51, Instruction{InstType::LD,   "ld d,c",     1,  4, Operand::D, Operand::C});
			map_inst.emplace(0x52, Instruction{InstType::LD,   "ld d,d",     1,  4, Operand::D, Operand::D});
			map_inst.emplace(0x53, Instruction{InstType::LD,   "ld d,e",     1,  4, Operand::D, Operand::E});
			map_inst.emplace(0x54, Instruction{InstType::LD,   "ld d,h",     1,  4, Operand::D, Operand::H});
			map_inst.emplace(0x55, Instruction{InstType::LD,   "ld d,l",     1,  4, Operand::D, Operand::L});
			map_inst.emplace(0x56, Instruction{InstType::LD,   "ld d,(hl)",  1,  7, Operand::D, Operand::indHL});
			map_inst.emplace(0x57, Instruction{InstType::LD,   "ld d,a",     1,  4, Operand::D, Operand::A});
			map_inst.emplace(0x58, Instruction{InstType::LD,   "ld e,b",     1,  4, Operand::E, Operand::B});
			map_inst.emplace(0x59, Instruction{InstType::LD,   "ld e,c",     1,  4, Operand::E, Operand::C});
			map_inst.emplace(0x5a, Instruction{InstType::LD,   "ld e,d",     1,  4, Operand::E, Operand::D});
			map_inst.emplace(0x5b, Instruction{InstType::LD,   "ld e,e",     1,  4, Operand::E, Operand::E});
			map_inst.emplace(0x5c, Instruction{InstType::LD,   "ld e,h",     1,  4, Operand::E, Operand::H});
			map_inst.emplace(0x5d, Instruction{InstType::LD,   "ld e,l",     1,  4, Operand::E, Operand::L});
			map_inst.emplace(0x5e, Instruction{InstType::LD,   "ld e,(hl)",  1,  7, Operand::E, Operand::indHL});
			map_inst.emplace(0x5f, Instruction{InstType::LD,   "ld e,a",     1,  4, Operand::E, Operand::A});
			map_inst.emplace(0x60, Instruction{InstType::LD,   "ld h,b",     1,  4, Operand::H, Operand::B});
			map_inst.emplace(0x61, Instruction{InstType::LD,   "ld h,c",     1,  4, Operand::H, Operand::C});
			map_inst.emplace(0x62, Instruction{InstType::LD,   "ld h,d",     1,  4, Operand::H, Operand::D});
			map_inst.emplace(0x63, Instruction{InstType::LD,   "ld h,e",     1,  4, Operand::H, Operand::E});
			map_inst.emplace(0x64, Instruction{InstType::LD,   "ld h,h",     1,  4, Operand::H, Operand::H});
			map_inst.emplace(0x65, Instruction{InstType::LD,   "ld h,l",     1,  4, Operand::H, Operand::L});
			map_inst.emplace(0x66, Instruction{InstType::LD,   "ld h,(hl)",  1,  7, Operand::H, Operand::indHL});
			map_inst.emplace(0x67, Instruction{InstType::LD,   "ld h,a",     1,  4, Operand::H, Operand::A});
			map_inst.emplace(0x68, Instruction{InstType::LD,   "ld l,b",     1,  4, Operand::L, Operand::B});
			map_inst.emplace(0x69, Instruction{InstType::LD,   "ld l,c",     1,  4, Operand::L, Operand::C});
			map_inst.emplace(0x6a, Instruction{InstType::LD,   "ld l,d",     1,  4, Operand::L, Operand::D});
			map_inst.emplace(0x6b, Instruction{InstType::LD,   "ld l,e",     1,  4, Operand::L, Operand::E});
			map_inst.emplace(0x6c, Instruction{InstType::LD,   "ld l,h",     1,  4, Operand::L, Operand::H});
			map_inst.emplace(0x6d, Instruction{InstType::LD,   "ld l,l",     1,  4, Operand::L, Operand::L});
			map_inst.emplace(0x6e, Instruction{InstType::LD,   "ld l,(hl)",  1,  7, Operand::L, Operand::indHL});
			map_inst.emplace(0x6f, Instruction{InstType::LD,   "ld l,a",     1,  4, Operand::L, Operand::A});
			map_inst.emplace(0x70, Instruction{InstType::LD,   "ld (hl),b",  1,  7, Operand::indHL, Operand::B});
			map_inst.emplace(0x71, Instruction{InstType::LD,   "ld (hl),c",  1,  7, Operand::indHL, Operand::C});
			map_inst.emplace(0x72, Instruction{InstType::LD,   "ld (hl),d",  1,  7, Operand::indHL, Operand::D});
			map_inst.emplace(0x73, Instruction{InstType::LD,   "ld (hl),e",  1,  7, Operand::indHL, Operand::E});
			map_inst.emplace(0x74, Instruction{InstType::LD,   "ld (hl),h",  1,  7, Operand::indHL, Operand::H});
			map_inst.emplace(0x75, Instruction{InstType::LD,   "ld (hl),l",  1,  7, Operand::indHL, Operand::L});
			map_inst.emplace(0x77, Instruction{InstType::LD,   "ld (hl),a",  1,  7, Operand::indHL, Operand::A});
			map_inst.emplace(0x78, Instruction{InstType::LD,   "ld a,b",     1,  4, Operand::A, Operand::B});
			map_inst.emplace(0x79, Instruction{InstType::LD,   "ld a,c",     1,  4, Operand::A, Operand::C});
			map_inst.emplace(0x7a, Instruction{InstType::LD,   "ld a,d",     1,  4, Operand::A, Operand::D});
			map_inst.emplace(0x7b, Instruction{InstType::LD,   "ld a,e",     1,  4, Operand::A, Operand::E});
			map_inst.emplace(0x7c, Instruction{InstType::LD,   "ld a,h",     1,  4, Operand::A, Operand::H});
			map_inst.emplace(0x7d, Instruction{InstType::LD,   "ld a,l",     1,  4, Operand::A, Operand::L});
			map_inst.emplace(0x7e, Instruction{InstType::LD,   "ld a,(hl)",  1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0x7f, Instruction{InstType::LD,   "ld a,a",     1,  4, Operand::A, Operand::A});
			map_inst.emplace(0x80, Instruction{InstType::ADD,  "add b",      1,  4, Operand::A, Operand::B});
			map_inst.emplace(0x81, Instruction{InstType::ADD,  "add c",      1,  4, Operand::A, Operand::C});
			map_inst.emplace(0x82, Instruction{InstType::ADD,  "add d",      1,  4, Operand::A, Operand::D});
			map_inst.emplace(0x83, Instruction{InstType::ADD,  "add e",      1,  4, Operand::A, Operand::E});
			map_inst.emplace(0x84, Instruction{InstType::ADD,  "add h",      1,  4, Operand::A, Operand::H});
			map_inst.emplace(0x85, Instruction{InstType::ADD,  "add l",      1,  4, Operand::A, Operand::L});
			map_inst.emplace(0x86, Instruction{InstType::ADD,  "add (hl)",   1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0x87, Instruction{InstType::ADD,  "add a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0x90, Instruction{InstType::SUB,  "sub b",      1,  4, Operand::A, Operand::B});
			map_inst.emplace(0x91, Instruction{InstType::SUB,  "sub c",      1,  4, Operand::A, Operand::C});
			map_inst.emplace(0x92, Instruction{InstType::SUB,  "sub d",      1,  4, Operand::A, Operand::D});
			map_inst.emplace(0x93, Instruction{InstType::SUB,  "sub e",      1,  4, Operand::A, Operand::E});
			map_inst.emplace(0x94, Instruction{InstType::SUB,  "sub h",      1,  4, Operand::A, Operand::H});
			map_inst.emplace(0x95, Instruction{InstType::SUB,  "sub l",      1,  4, Operand::A, Operand::L});
			map_inst.emplace(0x96, Instruction{InstType::SUB,  "sub (hl)",   1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0x97, Instruction{InstType::SUB,  "sub a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xa0, Instruction{InstType::AND,  "and b",      1,  4, Operand::A, Operand::B});
			map_inst.emplace(0xa1, Instruction{InstType::AND,  "and c",      1,  4, Operand::A, Operand::C});
			map_inst.emplace(0xa2, Instruction{InstType::AND,  "and d",      1,  4, Operand::A, Operand::D});
			map_inst.emplace(0xa3, Instruction{InstType::AND,  "and e",      1,  4, Operand::A, Operand::E});
			map_inst.emplace(0xa4, Instruction{InstType::AND,  "and h",      1,  4, Operand::A, Operand::H});
			map_inst.emplace(0xa5, Instruction{InstType::AND,  "and l",      1,  4, Operand::A, Operand::L});
			map_inst.emplace(0xa6, Instruction{InstType::AND,  "and (hl)",   1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0xa7, Instruction{InstType::AND,  "and a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xa8, Instruction{InstType::XOR,  "xor b",      1,  4, Operand::A, Operand::B});
			map_inst.emplace(0xa9, Instruction{InstType::XOR,  "xor c",      1,  4, Operand::A, Operand::C});
			map_inst.emplace(0xaa, Instruction{InstType::XOR,  "xor d",      1,  4, Operand::A, Operand::D});
			map_inst.emplace(0xab, Instruction{InstType::XOR,  "xor e",      1,  4, Operand::A, Operand::E});
			map_inst.emplace(0xac, Instruction{InstType::XOR,  "xor h",      1,  4, Operand::A, Operand::H});
			map_inst.emplace(0xad, Instruction{InstType::XOR,  "xor l",      1,  4, Operand::A, Operand::L});
			map_inst.emplace(0xae, Instruction{InstType::XOR,  "xor (hl)",   1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0xaf, Instruction{InstType::XOR,  "xor a",      1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xb0, Instruction{InstType::OR,   "or b",       1,  4, Operand::A, Operand::B});
			map_inst.emplace(0xb1, Instruction{InstType::OR,   "or c",       1,  4, Operand::A, Operand::C});
			map_inst.emplace(0xb2, Instruction{InstType::OR,   "or d",       1,  4, Operand::A, Operand::D});
			map_inst.emplace(0xb3, Instruction{InstType::OR,   "or e",       1,  4, Operand::A, Operand::E});
			map_inst.emplace(0xb4, Instruction{InstType::OR,   "or h",       1,  4, Operand::A, Operand::H});
			map_inst.emplace(0xb5, Instruction{InstType::OR,   "or l",       1,  4, Operand::A, Operand::L});
			map_inst.emplace(0xb6, Instruction{InstType::OR,   "or (hl)",    1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0xb7, Instruction{InstType::OR,   "or a",       1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xb8, Instruction{InstType::CP,   "cp b",       1,  4, Operand::A, Operand::B});
			map_inst.emplace(0xb9, Instruction{InstType::CP,   "cp c",       1,  4, Operand::A, Operand::C});
			map_inst.emplace(0xba, Instruction{InstType::CP,   "cp d",       1,  4, Operand::A, Operand::D});
			map_inst.emplace(0xbb, Instruction{InstType::CP,   "cp e",       1,  4, Operand::A, Operand::E});
			map_inst.emplace(0xbc, Instruction{InstType::CP,   "cp h",       1,  4, Operand::A, Operand::H});
			map_inst.emplace(0xbd, Instruction{InstType::CP,   "cp l",       1,  4, Operand::A, Operand::L});
			map_inst.emplace(0xbe, Instruction{InstType::CP,   "cp (hl)",    1,  7, Operand::A, Operand::indHL});
			map_inst.emplace(0xbf, Instruction{InstType::CP,   "cp a",       1,  4, Operand::A, Operand::A});
			map_inst.emplace(0xc0, Instruction{InstType::RET,  "ret nz",     1, 11, 5, Conditional::NZ, Operand::PC});
			map_inst.emplace(0xc1, Instruction{InstType::POP,  "pop bc",     1, 10, Operand::BC});
			map_inst.emplace(0xc2, Instruction{InstType::JP,   "jp nz,**",   3, 10, Conditional::NZ, Operand::PC, Operand::NN});
			map_inst.emplace(0xc3, Instruction{InstType::JP,   "jp **",      3, 10, Conditional::ALWAYS, Operand::PC, Operand::NN});
			map_inst.emplace(0xc4, Instruction{InstType::CALL, "call nz,**", 3, 17, 10, Conditional::NZ, Operand::PC, Operand::NN});
			map_inst.emplace(0xc5, Instruction{InstType::PUSH, "push bc",    1, 11, Operand::UNUSED, Operand::BC});
			map_inst.emplace(0xc6, Instruction{InstType::ADD,  "add a,*",    2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xc7, Instruction{InstType::RST,  "rst 00h",    1, 11, Operand::PC, Operand::HEX_0000});
			map_inst.emplace(0xc8, Instruction{InstType::RET,  "ret z",      1, 11, 5, Conditional::Z, Operand::PC});
			map_inst.emplace(0xc9, Instruction{InstType::RET,  "ret",        1, 10, Conditional::ALWAYS, Operand::PC});
			map_inst.emplace(0xca, Instruction{InstType::JP,   "jp z,**",    3, 10, Conditional::Z, Operand::PC, Operand::NN});
			map_inst.emplace(0xcc, Instruction{InstType::CALL, "call z,**",  3, 17, 10, Conditional::Z, Operand::PC, Operand::NN});
			map_inst.emplace(0xcd, Instruction{InstType::CALL, "call **",    3, 17, Conditional::ALWAYS, Operand::PC, Operand::NN});
			map_inst.emplace(0xcf, Instruction{InstType::RST,  "rst 08h",    1, 11, Operand::PC, Operand::HEX_0008});
			map_inst.emplace(0xd0, Instruction{InstType::RET,  "ret nc",     1, 11, 5, Conditional::NC, Operand::PC});
			map_inst.emplace(0xd1, Instruction{InstType::POP,  "pop de",     1, 10, Operand::DE});
			map_inst.emplace(0xd2, Instruction{InstType::JP,   "jp nc,**",   3, 10, Conditional::NC, Operand::PC, Operand::NN});
			map_inst.emplace(0xd3, Instruction{InstType::OUT,  "out (*),a",  2, 11, Operand::PORT, Operand::A});
			map_inst.emplace(0xd4, Instruction{InstType::CALL, "call nc,**", 3, 17, 10, Conditional::NC, Operand::PC, Operand::NN});
			map_inst.emplace(0xd5, Instruction{InstType::PUSH, "push de",    1, 11, Operand::UNUSED, Operand::DE});
			map_inst.emplace(0xd6, Instruction{InstType::SUB,  "sub *",      2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xd7, Instruction{InstType::RST,  "rst 10h",    1, 11, Operand::PC, Operand::HEX_0010});
			map_inst.emplace(0xd8, Instruction{InstType::RET,  "ret c",      1, 11, 5, Conditional::C, Operand::PC});
			map_inst.emplace(0xd9, Instruction{InstType::EX,   "exx",        1,  4});
			map_inst.emplace(0xda, Instruction{InstType::JP,   "jp c,**",    3, 10, Conditional::C, Operand::PC, Operand::NN});
			map_inst.emplace(0xdc, Instruction{InstType::CALL, "call c,**",  3, 17, 10, Conditional::C, Operand::PC, Operand::NN});
			map_inst.emplace(0xdf, Instruction{InstType::RST,  "rst 18h",    1, 11, Operand::PC, Operand::HEX_0018});
			map_inst.emplace(0xe0, Instruction{InstType::RET,  "ret po",     1, 11, 5, Conditional::PO, Operand::PC});
			map_inst.emplace(0xe1, Instruction{InstType::POP,  "pop hl",     1, 10, Operand::HL});
			map_inst.emplace(0xe2, Instruction{InstType::JP,   "jp po,**",   3, 10, Conditional::PO, Operand::PC, Operand::NN});
			map_inst.emplace(0xe3, Instruction{InstType::EX,   "ex (sp),hl", 1, 19, Operand::indSP, Operand::HL});
			map_inst.emplace(0xe4, Instruction{InstType::CALL, "call po,**", 3, 17, 10, Conditional::PO, Operand::PC, Operand::NN});
			map_inst.emplace(0xe5, Instruction{InstType::PUSH, "push hl",    1, 11, Operand::UNUSED, Operand::HL});
			map_inst.emplace(0xe6, Instruction{InstType::AND,  "and *",      2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xe7, Instruction{InstType::RST,  "rst 20h",    1, 11, Operand::PC, Operand::HEX_0020});
			map_inst.emplace(0xe8, Instruction{InstType::RET,  "ret pe",     1, 11, 5, Conditional::PE, Operand::PC});
			map_inst.emplace(0xe9, Instruction{InstType::JP,   "jp (hl)",    1,  4, Conditional::ALWAYS, Operand::PC, Operand::HL});
			map_inst.emplace(0xea, Instruction{InstType::JP,   "jp pe,**",   3, 10, Conditional::PE, Operand::PC, Operand::NN});
			map_inst.emplace(0xeb, Instruction{InstType::EX,   "ex de,hl",   1,  4, Operand::DE, Operand::HL});
			map_inst.emplace(0xec, Instruction{InstType::CALL, "call pe,**", 3, 17, 10, Conditional::PE, Operand::PC, Operand::NN});
			map_inst.emplace(0xee, Instruction{InstType::XOR,  "xor *",      2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xef, Instruction{InstType::RST,  "rst 28h",    1, 11, Operand::PC, Operand::HEX_0028});
			map_inst.emplace(0xf0, Instruction{InstType::RET,  "ret p",      1, 11, 5, Conditional::P, Operand::PC});
			map_inst.emplace(0xf1, Instruction{InstType::POP,  "pop af",     1, 10, Operand::AF});
			map_inst.emplace(0xf2, Instruction{InstType::JP,   "jp p,**",    3, 10, Conditional::P, Operand::PC, Operand::NN});
			map_inst.emplace(0xf3, Instruction{InstType::DI,   "di",         1,  4});
			map_inst.emplace(0xf4, Instruction{InstType::CALL, "call p,**",  3, 17, 10, Conditional::P, Operand::PC, Operand::NN});
			map_inst.emplace(0xf5, Instruction{InstType::PUSH, "push af",    1, 11, Operand::UNUSED, Operand::AF});
			map_inst.emplace(0xf6, Instruction{InstType::OR,   "or *",       2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xf7, Instruction{InstType::RST,  "rst 30h",    1, 11, Operand::PC, Operand::HEX_0030});
			map_inst.emplace(0xf8, Instruction{InstType::RET,  "ret m",      1, 11, 5, Conditional::M, Operand::PC});
			map_inst.emplace(0xf9, Instruction{InstType::LD,   "ld sp,hl",   1,  6, Operand::SP, Operand::HL});
			map_inst.emplace(0xfa, Instruction{InstType::JP,   "jp m,**",    3, 10, Conditional::M, Operand::PC, Operand::NN});
			map_inst.emplace(0xfb, Instruction{InstType::EI,   "ei",         1,  4});
			map_inst.emplace(0xfc, Instruction{InstType::CALL, "call m,**",  3, 17, 10, Conditional::M, Operand::PC, Operand::NN});
			map_inst.emplace(0xfe, Instruction{InstType::CP,   "cp *",       2,  7, Operand::A, Operand::N});
			map_inst.emplace(0xff, Instruction{InstType::RST,  "rst 38h",    1, 11, Operand::PC, Operand::HEX_0038});

			map_inst.emplace(0xcb40, Instruction{InstType::BIT,  "bit 0,b",    2,  8, Operand::B, Operand::ZERO});
			map_inst.emplace(0xcb41, Instruction{InstType::BIT,  "bit 0,c",    2,  8, Operand::C, Operand::ZERO});
			map_inst.emplace(0xcb42, Instruction{InstType::BIT,  "bit 0,d",    2,  8, Operand::D, Operand::ZERO});
			map_inst.emplace(0xcb43, Instruction{InstType::BIT,  "bit 0,e",    2,  8, Operand::E, Operand::ZERO});
			map_inst.emplace(0xcb44, Instruction{InstType::BIT,  "bit 0,h",    2,  8, Operand::H, Operand::ZERO});
			map_inst.emplace(0xcb45, Instruction{InstType::BIT,  "bit 0,l",    2,  8, Operand::L, Operand::ZERO});
			map_inst.emplace(0xcb46, Instruction{InstType::BIT,  "bit 0,(hl)", 2, 12, Operand::indHL, Operand::ZERO});
			map_inst.emplace(0xcb47, Instruction{InstType::BIT,  "bit 0,a",    2,  8, Operand::A, Operand::ZERO});
			map_inst.emplace(0xcb48, Instruction{InstType::BIT,  "bit 1,b",    2,  8, Operand::B, Operand::ONE});
			map_inst.emplace(0xcb49, Instruction{InstType::BIT,  "bit 1,c",    2,  8, Operand::C, Operand::ONE});
			map_inst.emplace(0xcb4a, Instruction{InstType::BIT,  "bit 1,d",    2,  8, Operand::D, Operand::ONE});
			map_inst.emplace(0xcb4b, Instruction{InstType::BIT,  "bit 1,e",    2,  8, Operand::E, Operand::ONE});
			map_inst.emplace(0xcb4c, Instruction{InstType::BIT,  "bit 1,h",    2,  8, Operand::H, Operand::ONE});
			map_inst.emplace(0xcb4d, Instruction{InstType::BIT,  "bit 1,l",    2,  8, Operand::L, Operand::ONE});
			map_inst.emplace(0xcb4e, Instruction{InstType::BIT,  "bit 1,(hl)", 2, 12, Operand::indHL, Operand::ONE});
			map_inst.emplace(0xcb4f, Instruction{InstType::BIT,  "bit 1,a",    2,  8, Operand::A, Operand::ONE});
			map_inst.emplace(0xcb50, Instruction{InstType::BIT,  "bit 2,b",    2,  8, Operand::B, Operand::TWO});
			map_inst.emplace(0xcb51, Instruction{InstType::BIT,  "bit 2,c",    2,  8, Operand::C, Operand::TWO});
			map_inst.emplace(0xcb52, Instruction{InstType::BIT,  "bit 2,d",    2,  8, Operand::D, Operand::TWO});
			map_inst.emplace(0xcb53, Instruction{InstType::BIT,  "bit 2,e",    2,  8, Operand::E, Operand::TWO});
			map_inst.emplace(0xcb54, Instruction{InstType::BIT,  "bit 2,h",    2,  8, Operand::H, Operand::TWO});
			map_inst.emplace(0xcb55, Instruction{InstType::BIT,  "bit 2,l",    2,  8, Operand::L, Operand::TWO});
			map_inst.emplace(0xcb56, Instruction{InstType::BIT,  "bit 2,(hl)", 2, 12, Operand::indHL, Operand::TWO});
			map_inst.emplace(0xcb57, Instruction{InstType::BIT,  "bit 2,a",    2,  8, Operand::A, Operand::TWO});
			map_inst.emplace(0xcb58, Instruction{InstType::BIT,  "bit 3,b",    2,  8, Operand::B, Operand::THREE});
			map_inst.emplace(0xcb59, Instruction{InstType::BIT,  "bit 3,c",    2,  8, Operand::C, Operand::THREE});
			map_inst.emplace(0xcb5a, Instruction{InstType::BIT,  "bit 3,d",    2,  8, Operand::D, Operand::THREE});
			map_inst.emplace(0xcb5b, Instruction{InstType::BIT,  "bit 3,e",    2,  8, Operand::E, Operand::THREE});
			map_inst.emplace(0xcb5c, Instruction{InstType::BIT,  "bit 3,h",    2,  8, Operand::H, Operand::THREE});
			map_inst.emplace(0xcb5d, Instruction{InstType::BIT,  "bit 3,l",    2,  8, Operand::L, Operand::THREE});
			map_inst.emplace(0xcb5e, Instruction{InstType::BIT,  "bit 3,(hl)", 2, 12, Operand::indHL, Operand::THREE});
			map_inst.emplace(0xcb5f, Instruction{InstType::BIT,  "bit 3,a",    2,  8, Operand::A, Operand::THREE});
			map_inst.emplace(0xcb60, Instruction{InstType::BIT,  "bit 4,b",    2,  8, Operand::B, Operand::FOUR});
			map_inst.emplace(0xcb61, Instruction{InstType::BIT,  "bit 4,c",    2,  8, Operand::C, Operand::FOUR});
			map_inst.emplace(0xcb62, Instruction{InstType::BIT,  "bit 4,d",    2,  8, Operand::D, Operand::FOUR});
			map_inst.emplace(0xcb63, Instruction{InstType::BIT,  "bit 4,e",    2,  8, Operand::E, Operand::FOUR});
			map_inst.emplace(0xcb64, Instruction{InstType::BIT,  "bit 4,h",    2,  8, Operand::H, Operand::FOUR});
			map_inst.emplace(0xcb65, Instruction{InstType::BIT,  "bit 4,l",    2,  8, Operand::L, Operand::FOUR});
			map_inst.emplace(0xcb66, Instruction{InstType::BIT,  "bit 4,(hl)", 2, 12, Operand::indHL, Operand::FOUR});
			map_inst.emplace(0xcb67, Instruction{InstType::BIT,  "bit 4,a",    2,  8, Operand::A, Operand::FOUR});
			map_inst.emplace(0xcb68, Instruction{InstType::BIT,  "bit 5,b",    2,  8, Operand::B, Operand::FIVE});
			map_inst.emplace(0xcb69, Instruction{InstType::BIT,  "bit 5,c",    2,  8, Operand::C, Operand::FIVE});
			map_inst.emplace(0xcb6a, Instruction{InstType::BIT,  "bit 5,d",    2,  8, Operand::D, Operand::FIVE});
			map_inst.emplace(0xcb6b, Instruction{InstType::BIT,  "bit 5,e",    2,  8, Operand::E, Operand::FIVE});
			map_inst.emplace(0xcb6c, Instruction{InstType::BIT,  "bit 5,h",    2,  8, Operand::H, Operand::FIVE});
			map_inst.emplace(0xcb6d, Instruction{InstType::BIT,  "bit 5,l",    2,  8, Operand::L, Operand::FIVE});
			map_inst.emplace(0xcb6e, Instruction{InstType::BIT,  "bit 5,(hl)", 2, 12, Operand::indHL, Operand::FIVE});
			map_inst.emplace(0xcb6f, Instruction{InstType::BIT,  "bit 5,a",    2,  8, Operand::A, Operand::FIVE});
			map_inst.emplace(0xcb70, Instruction{InstType::BIT,  "bit 6,b",    2,  8, Operand::B, Operand::SIX});
			map_inst.emplace(0xcb71, Instruction{InstType::BIT,  "bit 6,c",    2,  8, Operand::C, Operand::SIX});
			map_inst.emplace(0xcb72, Instruction{InstType::BIT,  "bit 6,d",    2,  8, Operand::D, Operand::SIX});
			map_inst.emplace(0xcb73, Instruction{InstType::BIT,  "bit 6,e",    2,  8, Operand::E, Operand::SIX});
			map_inst.emplace(0xcb74, Instruction{InstType::BIT,  "bit 6,h",    2,  8, Operand::H, Operand::SIX});
			map_inst.emplace(0xcb75, Instruction{InstType::BIT,  "bit 6,l",    2,  8, Operand::L, Operand::SIX});
			map_inst.emplace(0xcb76, Instruction{InstType::BIT,  "bit 6,(hl)", 2, 12, Operand::indHL, Operand::SIX});
			map_inst.emplace(0xcb77, Instruction{InstType::BIT,  "bit 6,a",    2,  8, Operand::A, Operand::SIX});
			map_inst.emplace(0xcb78, Instruction{InstType::BIT,  "bit 7,b",    2,  8, Operand::B, Operand::SEVEN});
			map_inst.emplace(0xcb79, Instruction{InstType::BIT,  "bit 7,c",    2,  8, Operand::C, Operand::SEVEN});
			map_inst.emplace(0xcb7a, Instruction{InstType::BIT,  "bit 7,d",    2,  8, Operand::D, Operand::SEVEN});
			map_inst.emplace(0xcb7b, Instruction{InstType::BIT,  "bit 7,e",    2,  8, Operand::E, Operand::SEVEN});
			map_inst.emplace(0xcb7c, Instruction{InstType::BIT,  "bit 7,h",    2,  8, Operand::H, Operand::SEVEN});
			map_inst.emplace(0xcb7d, Instruction{InstType::BIT,  "bit 7,l",    2,  8, Operand::L, Operand::SEVEN});
			map_inst.emplace(0xcb7e, Instruction{InstType::BIT,  "bit 7,(hl)", 2, 12, Operand::indHL, Operand::SEVEN});
			map_inst.emplace(0xcb7f, Instruction{InstType::BIT,  "bit 7,a",    2,  8, Operand::A, Operand::SEVEN});
			map_inst.emplace(0xcb80, Instruction{InstType::RES,  "res 0,b",    2,  8, Operand::B, Operand::ZERO});
			map_inst.emplace(0xcb81, Instruction{InstType::RES,  "res 0,c",    2,  8, Operand::C, Operand::ZERO});
			map_inst.emplace(0xcb82, Instruction{InstType::RES,  "res 0,d",    2,  8, Operand::D, Operand::ZERO});
			map_inst.emplace(0xcb83, Instruction{InstType::RES,  "res 0,e",    2,  8, Operand::E, Operand::ZERO});
			map_inst.emplace(0xcb84, Instruction{InstType::RES,  "res 0,h",    2,  8, Operand::H, Operand::ZERO});
			map_inst.emplace(0xcb85, Instruction{InstType::RES,  "res 0,l",    2,  8, Operand::L, Operand::ZERO});
			map_inst.emplace(0xcb86, Instruction{InstType::RES,  "res 0,(hl)", 2, 15, Operand::indHL, Operand::ZERO});
			map_inst.emplace(0xcb87, Instruction{InstType::RES,  "res 0,a",    2,  8, Operand::A, Operand::ZERO});
			map_inst.emplace(0xcb88, Instruction{InstType::RES,  "res 1,b",    2,  8, Operand::B, Operand::ONE});
			map_inst.emplace(0xcb89, Instruction{InstType::RES,  "res 1,c",    2,  8, Operand::C, Operand::ONE});
			map_inst.emplace(0xcb8a, Instruction{InstType::RES,  "res 1,d",    2,  8, Operand::D, Operand::ONE});
			map_inst.emplace(0xcb8b, Instruction{InstType::RES,  "res 1,e",    2,  8, Operand::E, Operand::ONE});
			map_inst.emplace(0xcb8c, Instruction{InstType::RES,  "res 1,h",    2,  8, Operand::H, Operand::ONE});
			map_inst.emplace(0xcb8d, Instruction{InstType::RES,  "res 1,l",    2,  8, Operand::L, Operand::ONE});
			map_inst.emplace(0xcb8e, Instruction{InstType::RES,  "res 1,(hl)", 2, 15, Operand::indHL, Operand::ONE});
			map_inst.emplace(0xcb8f, Instruction{InstType::RES,  "res 1,a",    2,  8, Operand::A, Operand::ONE});
			map_inst.emplace(0xcb90, Instruction{InstType::RES,  "res 2,b",    2,  8, Operand::B, Operand::TWO});
			map_inst.emplace(0xcb91, Instruction{InstType::RES,  "res 2,c",    2,  8, Operand::C, Operand::TWO});
			map_inst.emplace(0xcb92, Instruction{InstType::RES,  "res 2,d",    2,  8, Operand::D, Operand::TWO});
			map_inst.emplace(0xcb93, Instruction{InstType::RES,  "res 2,e",    2,  8, Operand::E, Operand::TWO});
			map_inst.emplace(0xcb94, Instruction{InstType::RES,  "res 2,h",    2,  8, Operand::H, Operand::TWO});
			map_inst.emplace(0xcb95, Instruction{InstType::RES,  "res 2,l",    2,  8, Operand::L, Operand::TWO});
			map_inst.emplace(0xcb96, Instruction{InstType::RES,  "res 2,(hl)", 2, 15, Operand::indHL, Operand::TWO});
			map_inst.emplace(0xcb97, Instruction{InstType::RES,  "res 2,a",    2,  8, Operand::A, Operand::TWO});
			map_inst.emplace(0xcb98, Instruction{InstType::RES,  "res 3,b",    2,  8, Operand::B, Operand::THREE});
			map_inst.emplace(0xcb99, Instruction{InstType::RES,  "res 3,c",    2,  8, Operand::C, Operand::THREE});
			map_inst.emplace(0xcb9a, Instruction{InstType::RES,  "res 3,d",    2,  8, Operand::D, Operand::THREE});
			map_inst.emplace(0xcb9b, Instruction{InstType::RES,  "res 3,e",    2,  8, Operand::E, Operand::THREE});
			map_inst.emplace(0xcb9c, Instruction{InstType::RES,  "res 3,h",    2,  8, Operand::H, Operand::THREE});
			map_inst.emplace(0xcb9d, Instruction{InstType::RES,  "res 3,l",    2,  8, Operand::L, Operand::THREE});
			map_inst.emplace(0xcb9e, Instruction{InstType::RES,  "res 3,(hl)", 2, 15, Operand::indHL, Operand::THREE});
			map_inst.emplace(0xcb9f, Instruction{InstType::RES,  "res 3,a",    2,  8, Operand::A, Operand::THREE});
			map_inst.emplace(0xcba0, Instruction{InstType::RES,  "res 4,b",    2,  8, Operand::B, Operand::FOUR});
			map_inst.emplace(0xcba1, Instruction{InstType::RES,  "res 4,c",    2,  8, Operand::C, Operand::FOUR});
			map_inst.emplace(0xcba2, Instruction{InstType::RES,  "res 4,d",    2,  8, Operand::D, Operand::FOUR});
			map_inst.emplace(0xcba3, Instruction{InstType::RES,  "res 4,e",    2,  8, Operand::E, Operand::FOUR});
			map_inst.emplace(0xcba4, Instruction{InstType::RES,  "res 4,h",    2,  8, Operand::H, Operand::FOUR});
			map_inst.emplace(0xcba5, Instruction{InstType::RES,  "res 4,l",    2,  8, Operand::L, Operand::FOUR});
			map_inst.emplace(0xcba6, Instruction{InstType::RES,  "res 4,(hl)", 2, 15, Operand::indHL, Operand::FOUR});
			map_inst.emplace(0xcba7, Instruction{InstType::RES,  "res 4,a",    2,  8, Operand::A, Operand::FOUR});
			map_inst.emplace(0xcba8, Instruction{InstType::RES,  "res 5,b",    2,  8, Operand::B, Operand::FIVE});
			map_inst.emplace(0xcba9, Instruction{InstType::RES,  "res 5,c",    2,  8, Operand::C, Operand::FIVE});
			map_inst.emplace(0xcbaa, Instruction{InstType::RES,  "res 5,d",    2,  8, Operand::D, Operand::FIVE});
			map_inst.emplace(0xcbab, Instruction{InstType::RES,  "res 5,e",    2,  8, Operand::E, Operand::FIVE});
			map_inst.emplace(0xcbac, Instruction{InstType::RES,  "res 5,h",    2,  8, Operand::H, Operand::FIVE});
			map_inst.emplace(0xcbad, Instruction{InstType::RES,  "res 5,l",    2,  8, Operand::L, Operand::FIVE});
			map_inst.emplace(0xcbae, Instruction{InstType::RES,  "res 5,(hl)", 2, 15, Operand::indHL, Operand::FIVE});
			map_inst.emplace(0xcbaf, Instruction{InstType::RES,  "res 5,a",    2,  8, Operand::A, Operand::FIVE});
			map_inst.emplace(0xcbb0, Instruction{InstType::RES,  "res 6,b",    2,  8, Operand::B, Operand::SIX});
			map_inst.emplace(0xcbb1, Instruction{InstType::RES,  "res 6,c",    2,  8, Operand::C, Operand::SIX});
			map_inst.emplace(0xcbb2, Instruction{InstType::RES,  "res 6,d",    2,  8, Operand::D, Operand::SIX});
			map_inst.emplace(0xcbb3, Instruction{InstType::RES,  "res 6,e",    2,  8, Operand::E, Operand::SIX});
			map_inst.emplace(0xcbb4, Instruction{InstType::RES,  "res 6,h",    2,  8, Operand::H, Operand::SIX});
			map_inst.emplace(0xcbb5, Instruction{InstType::RES,  "res 6,l",    2,  8, Operand::L, Operand::SIX});
			map_inst.emplace(0xcbb6, Instruction{InstType::RES,  "res 6,(hl)", 2, 15, Operand::indHL, Operand::SIX});
			map_inst.emplace(0xcbb7, Instruction{InstType::RES,  "res 6,a",    2,  8, Operand::A, Operand::SIX});
			map_inst.emplace(0xcbb8, Instruction{InstType::RES,  "res 7,b",    2,  8, Operand::B, Operand::SEVEN});
			map_inst.emplace(0xcbb9, Instruction{InstType::RES,  "res 7,c",    2,  8, Operand::C, Operand::SEVEN});
			map_inst.emplace(0xcbba, Instruction{InstType::RES,  "res 7,d",    2,  8, Operand::D, Operand::SEVEN});
			map_inst.emplace(0xcbbb, Instruction{InstType::RES,  "res 7,e",    2,  8, Operand::E, Operand::SEVEN});
			map_inst.emplace(0xcbbc, Instruction{InstType::RES,  "res 7,h",    2,  8, Operand::H, Operand::SEVEN});
			map_inst.emplace(0xcbbd, Instruction{InstType::RES,  "res 7,l",    2,  8, Operand::L, Operand::SEVEN});
			map_inst.emplace(0xcbbe, Instruction{InstType::RES,  "res 7,(hl)", 2, 15, Operand::indHL, Operand::SEVEN});
			map_inst.emplace(0xcbbf, Instruction{InstType::RES,  "res 7,a",    2,  8, Operand::A, Operand::SEVEN});
			map_inst.emplace(0xcbc0, Instruction{InstType::SET,  "set 0,b",    2,  8, Operand::B, Operand::ZERO});
			map_inst.emplace(0xcbc1, Instruction{InstType::SET,  "set 0,c",    2,  8, Operand::C, Operand::ZERO});
			map_inst.emplace(0xcbc2, Instruction{InstType::SET,  "set 0,d",    2,  8, Operand::D, Operand::ZERO});
			map_inst.emplace(0xcbc3, Instruction{InstType::SET,  "set 0,e",    2,  8, Operand::E, Operand::ZERO});
			map_inst.emplace(0xcbc4, Instruction{InstType::SET,  "set 0,h",    2,  8, Operand::H, Operand::ZERO});
			map_inst.emplace(0xcbc5, Instruction{InstType::SET,  "set 0,l",    2,  8, Operand::L, Operand::ZERO});
			map_inst.emplace(0xcbc6, Instruction{InstType::SET,  "set 0,(hl)", 2, 15, Operand::indHL, Operand::ZERO});
			map_inst.emplace(0xcbc7, Instruction{InstType::SET,  "set 0,a",    2,  8, Operand::A, Operand::ZERO});
			map_inst.emplace(0xcbc8, Instruction{InstType::SET,  "set 1,b",    2,  8, Operand::B, Operand::ONE});
			map_inst.emplace(0xcbc9, Instruction{InstType::SET,  "set 1,c",    2,  8, Operand::C, Operand::ONE});
			map_inst.emplace(0xcbca, Instruction{InstType::SET,  "set 1,d",    2,  8, Operand::D, Operand::ONE});
			map_inst.emplace(0xcbcb, Instruction{InstType::SET,  "set 1,e",    2,  8, Operand::E, Operand::ONE});
			map_inst.emplace(0xcbcc, Instruction{InstType::SET,  "set 1,h",    2,  8, Operand::H, Operand::ONE});
			map_inst.emplace(0xcbcd, Instruction{InstType::SET,  "set 1,l",    2,  8, Operand::L, Operand::ONE});
			map_inst.emplace(0xcbce, Instruction{InstType::SET,  "set 1,(hl)", 2, 15, Operand::indHL, Operand::ONE});
			map_inst.emplace(0xcbcf, Instruction{InstType::SET,  "set 1,a",    2,  8, Operand::A, Operand::ONE});
			map_inst.emplace(0xcbd0, Instruction{InstType::SET,  "set 2,b",    2,  8, Operand::B, Operand::TWO});
			map_inst.emplace(0xcbd1, Instruction{InstType::SET,  "set 2,c",    2,  8, Operand::C, Operand::TWO});
			map_inst.emplace(0xcbd2, Instruction{InstType::SET,  "set 2,d",    2,  8, Operand::D, Operand::TWO});
			map_inst.emplace(0xcbd3, Instruction{InstType::SET,  "set 2,e",    2,  8, Operand::E, Operand::TWO});
			map_inst.emplace(0xcbd4, Instruction{InstType::SET,  "set 2,h",    2,  8, Operand::H, Operand::TWO});
			map_inst.emplace(0xcbd5, Instruction{InstType::SET,  "set 2,l",    2,  8, Operand::L, Operand::TWO});
			map_inst.emplace(0xcbd6, Instruction{InstType::SET,  "set 2,(hl)", 2, 15, Operand::indHL, Operand::TWO});
			map_inst.emplace(0xcbd7, Instruction{InstType::SET,  "set 2,a",    2,  8, Operand::A, Operand::TWO});
			map_inst.emplace(0xcbd8, Instruction{InstType::SET,  "set 3,b",    2,  8, Operand::B, Operand::THREE});
			map_inst.emplace(0xcbd9, Instruction{InstType::SET,  "set 3,c",    2,  8, Operand::C, Operand::THREE});
			map_inst.emplace(0xcbda, Instruction{InstType::SET,  "set 3,d",    2,  8, Operand::D, Operand::THREE});
			map_inst.emplace(0xcbdb, Instruction{InstType::SET,  "set 3,e",    2,  8, Operand::E, Operand::THREE});
			map_inst.emplace(0xcbdc, Instruction{InstType::SET,  "set 3,h",    2,  8, Operand::H, Operand::THREE});
			map_inst.emplace(0xcbdd, Instruction{InstType::SET,  "set 3,l",    2,  8, Operand::L, Operand::THREE});
			map_inst.emplace(0xcbde, Instruction{InstType::SET,  "set 3,(hl)", 2, 15, Operand::indHL, Operand::THREE});
			map_inst.emplace(0xcbdf, Instruction{InstType::SET,  "set 3,a",    2,  8, Operand::A, Operand::THREE});
			map_inst.emplace(0xcbe0, Instruction{InstType::SET,  "set 4,b",    2,  8, Operand::B, Operand::FOUR});
			map_inst.emplace(0xcbe1, Instruction{InstType::SET,  "set 4,c",    2,  8, Operand::C, Operand::FOUR});
			map_inst.emplace(0xcbe2, Instruction{InstType::SET,  "set 4,d",    2,  8, Operand::D, Operand::FOUR});
			map_inst.emplace(0xcbe3, Instruction{InstType::SET,  "set 4,e",    2,  8, Operand::E, Operand::FOUR});
			map_inst.emplace(0xcbe4, Instruction{InstType::SET,  "set 4,h",    2,  8, Operand::H, Operand::FOUR});
			map_inst.emplace(0xcbe5, Instruction{InstType::SET,  "set 4,l",    2,  8, Operand::L, Operand::FOUR});
			map_inst.emplace(0xcbe6, Instruction{InstType::SET,  "set 4,(hl)", 2, 15, Operand::indHL, Operand::FOUR});
			map_inst.emplace(0xcbe7, Instruction{InstType::SET,  "set 4,a",    2,  8, Operand::A, Operand::FOUR});
			map_inst.emplace(0xcbe8, Instruction{InstType::SET,  "set 5,b",    2,  8, Operand::B, Operand::FIVE});
			map_inst.emplace(0xcbe9, Instruction{InstType::SET,  "set 5,c",    2,  8, Operand::C, Operand::FIVE});
			map_inst.emplace(0xcbea, Instruction{InstType::SET,  "set 5,d",    2,  8, Operand::D, Operand::FIVE});
			map_inst.emplace(0xcbeb, Instruction{InstType::SET,  "set 5,e",    2,  8, Operand::E, Operand::FIVE});
			map_inst.emplace(0xcbec, Instruction{InstType::SET,  "set 5,h",    2,  8, Operand::H, Operand::FIVE});
			map_inst.emplace(0xcbed, Instruction{InstType::SET,  "set 5,l",    2,  8, Operand::L, Operand::FIVE});
			map_inst.emplace(0xcbee, Instruction{InstType::SET,  "set 5,(hl)", 2, 15, Operand::indHL, Operand::FIVE});
			map_inst.emplace(0xcbef, Instruction{InstType::SET,  "set 5,a",    2,  8, Operand::A, Operand::FIVE});
			map_inst.emplace(0xcbf0, Instruction{InstType::SET,  "set 6,b",    2,  8, Operand::B, Operand::SIX});
			map_inst.emplace(0xcbf1, Instruction{InstType::SET,  "set 6,c",    2,  8, Operand::C, Operand::SIX});
			map_inst.emplace(0xcbf2, Instruction{InstType::SET,  "set 6,d",    2,  8, Operand::D, Operand::SIX});
			map_inst.emplace(0xcbf3, Instruction{InstType::SET,  "set 6,e",    2,  8, Operand::E, Operand::SIX});
			map_inst.emplace(0xcbf4, Instruction{InstType::SET,  "set 6,h",    2,  8, Operand::H, Operand::SIX});
			map_inst.emplace(0xcbf5, Instruction{InstType::SET,  "set 6,l",    2,  8, Operand::L, Operand::SIX});
			map_inst.emplace(0xcbf6, Instruction{InstType::SET,  "set 6,(hl)", 2, 15, Operand::indHL, Operand::SIX});
			map_inst.emplace(0xcbf7, Instruction{InstType::SET,  "set 6,a",    2,  8, Operand::A, Operand::SIX});
			map_inst.emplace(0xcbf8, Instruction{InstType::SET,  "set 7,b",    2,  8, Operand::B, Operand::SEVEN});
			map_inst.emplace(0xcbf9, Instruction{InstType::SET,  "set 7,c",    2,  8, Operand::C, Operand::SEVEN});
			map_inst.emplace(0xcbfa, Instruction{InstType::SET,  "set 7,d",    2,  8, Operand::D, Operand::SEVEN});
			map_inst.emplace(0xcbfb, Instruction{InstType::SET,  "set 7,e",    2,  8, Operand::E, Operand::SEVEN});
			map_inst.emplace(0xcbfc, Instruction{InstType::SET,  "set 7,h",    2,  8, Operand::H, Operand::SEVEN});
			map_inst.emplace(0xcbfd, Instruction{InstType::SET,  "set 7,l",    2,  8, Operand::L, Operand::SEVEN});
			map_inst.emplace(0xcbfe, Instruction{InstType::SET,  "set 7,(hl)", 2, 15, Operand::indHL, Operand::SEVEN});
			map_inst.emplace(0xcbff, Instruction{InstType::SET,  "set 7,a",    2,  8, Operand::A, Operand::SEVEN});

			map_inst.emplace(0xed43, Instruction{InstType::LD,   "ld (**),bc", 4, 20, Operand::indNN, Operand::BC});
			map_inst.emplace(0xed47, Instruction{InstType::LD,   "ld i,a",     2,  9, Operand::I, Operand::A});
			map_inst.emplace(0xed4b, Instruction{InstType::LD,   "ld bc,(**)", 4, 20, Operand::BC, Operand::indNN});
			map_inst.emplace(0xed4f, Instruction{InstType::LD,   "ld r,a",     2,  9, Operand::R, Operand::A});
			map_inst.emplace(0xed52, Instruction{InstType::SBC,  "sbc hl,de",  2, 15, Operand::HL, Operand::DE});
			map_inst.emplace(0xed53, Instruction{InstType::LD,   "ld (**),de", 4, 20, Operand::indNN, Operand::DE});
			map_inst.emplace(0xed56, Instruction{InstType::IM,   "im 1",       2,  8, Operand::IM, Operand::ONE});
			map_inst.emplace(0xed57, Instruction{InstType::LD,   "ld a,i",     2,  9, Operand::A, Operand::I});
			map_inst.emplace(0xed5b, Instruction{InstType::LD,   "ld de,(**)", 4, 20, Operand::DE, Operand::indNN});
			map_inst.emplace(0xed5f, Instruction{InstType::LD,   "ld a,r",     2,  9, Operand::A, Operand::R});
			map_inst.emplace(0xed73, Instruction{InstType::LD,   "ld (**),sp", 4, 20, Operand::indNN, Operand::SP});
			map_inst.emplace(0xed7b, Instruction{InstType::LD,   "ld sp,(**)", 4, 20, Operand::SP, Operand::indNN});
			map_inst.emplace(0xedb0, Instruction{InstType::LDIR, "ldir",       2, 21, 16, Operand::indDE, Operand::indHL});
			map_inst.emplace(0xedb8, Instruction{InstType::LDDR, "lddr",       2, 21, 16, Operand::indDE, Operand::indHL});

			map_inst.emplace(0xdd09, Instruction{InstType::ADD,  "add ix,bc",    2, 15, Operand::IX, Operand::BC});
			map_inst.emplace(0xdd19, Instruction{InstType::ADD,  "add ix,de",    2, 15, Operand::IX, Operand::DE});
			map_inst.emplace(0xdd21, Instruction{InstType::LD,   "ld ix,**",     4, 14, Operand::IX, Operand::NN});
			map_inst.emplace(0xdd22, Instruction{InstType::LD,   "ld (**),ix",   4, 20, Operand::indNN, Operand::IX});
			map_inst.emplace(0xdd23, Instruction{InstType::INC,  "inc ix",       2, 10, Operand::IX, Operand::ONE});
			map_inst.emplace(0xdd29, Instruction{InstType::ADD,  "add ix,ix",    2, 15, Operand::IX, Operand::IX});
			map_inst.emplace(0xdd2a, Instruction{InstType::LD,   "ld ix,(**)",   4, 20, Operand::IX, Operand::indNN});
			map_inst.emplace(0xdd2d, Instruction{InstType::DEC,  "dec ix",       2, 10, Operand::IX, Operand::ONE});
			map_inst.emplace(0xdd34, Instruction{InstType::INC,  "inc (ix+*)",   3, 23, Operand::indIXN, Operand::ONE});
			map_inst.emplace(0xdd35, Instruction{InstType::DEC,  "dec (ix+*)",   3, 23, Operand::indIXN, Operand::ONE});
			map_inst.emplace(0xdd36, Instruction{InstType::LD,   "ld (ix+*),*",  4, 19, Operand::indIXN, Operand::N});
			map_inst.emplace(0xdd39, Instruction{InstType::ADD,  "add ix,sp",    2, 15, Operand::IX, Operand::SP});
			map_inst.emplace(0xdd46, Instruction{InstType::LD,   "ld b,(ix+*)",  3, 19, Operand::B, Operand::indIXN});
			map_inst.emplace(0xdd4e, Instruction{InstType::LD,   "ld c,(ix+*)",  3, 19, Operand::C, Operand::indIXN});
			map_inst.emplace(0xdd56, Instruction{InstType::LD,   "ld d,(ix+*)",  3, 19, Operand::D, Operand::indIXN});
			map_inst.emplace(0xdd5e, Instruction{InstType::LD,   "ld e,(ix+*)",  3, 19, Operand::E, Operand::indIXN});
			map_inst.emplace(0xdd66, Instruction{InstType::LD,   "ld h,(ix+*)",  3, 19, Operand::H, Operand::indIXN});
			map_inst.emplace(0xdd6e, Instruction{InstType::LD,   "ld l,(ix+*)",  3, 19, Operand::L, Operand::indIXN});
			map_inst.emplace(0xdd70, Instruction{InstType::LD,   "ld (ix+*),b",  3, 19, Operand::indIXN, Operand::B});
			map_inst.emplace(0xdd71, Instruction{InstType::LD,   "ld (ix+*),c",  3, 19, Operand::indIXN, Operand::C});
			map_inst.emplace(0xdd72, Instruction{InstType::LD,   "ld (ix+*),d",  3, 19, Operand::indIXN, Operand::D});
			map_inst.emplace(0xdd73, Instruction{InstType::LD,   "ld (ix+*),e",  3, 19, Operand::indIXN, Operand::E});
			map_inst.emplace(0xdd74, Instruction{InstType::LD,   "ld (ix+*),h",  3, 19, Operand::indIXN, Operand::H});
			map_inst.emplace(0xdd75, Instruction{InstType::LD,   "ld (ix+*),l",  3, 19, Operand::indIXN, Operand::L});
			map_inst.emplace(0xdd77, Instruction{InstType::LD,   "ld (ix+*),a",  3, 19, Operand::indIXN, Operand::A});
			map_inst.emplace(0xdd7e, Instruction{InstType::LD,   "ld a,(ix+*)",  3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xdd86, Instruction{InstType::ADD,  "add a,(ix+*)", 3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xdd96, Instruction{InstType::SUB,  "sub (ix+*)",   3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xdda6, Instruction{InstType::AND,  "and (ix+*)",   3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xddae, Instruction{InstType::XOR,  "xor (ix+*)",   3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xddb6, Instruction{InstType::OR,   "or (ix+*)",    3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xddbe, Instruction{InstType::CP,   "cp (ix+*)",    3, 19, Operand::A, Operand::indIXN});
			map_inst.emplace(0xdde5, Instruction{InstType::PUSH, "push ix",      2, 15, Operand::UNUSED, Operand::IX});
			map_inst.emplace(0xdde1, Instruction{InstType::POP,  "pop ix",       2, 14, Operand::IX});
			map_inst.emplace(0xdde9, Instruction{InstType::JP,   "jp (ix)",      2,  8, Conditional::ALWAYS, Operand::PC, Operand::IX});
			map_inst.emplace(0xddf9, Instruction{InstType::LD,   "ld sp,ix",     2, 10, Operand::SP, Operand::IX});

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

			map_inst.emplace(0xfd09, Instruction{InstType::ADD,  "add iy,bc",    2, 15, Operand::IY, Operand::BC});
			map_inst.emplace(0xfd19, Instruction{InstType::ADD,  "add iy,de",    2, 15, Operand::IY, Operand::DE});
			map_inst.emplace(0xfd21, Instruction{InstType::LD,   "ld iy,**",     4, 14, Operand::IY, Operand::NN});
			map_inst.emplace(0xfd22, Instruction{InstType::LD,   "ld (**),iy",   4, 20, Operand::indNN, Operand::IY});
			map_inst.emplace(0xfd23, Instruction{InstType::INC,  "inc iy",       2, 10, Operand::IY, Operand::ONE});
			map_inst.emplace(0xfd29, Instruction{InstType::ADD,  "add iy,iy",    2, 15, Operand::IY, Operand::IY});
			map_inst.emplace(0xfd2a, Instruction{InstType::LD,   "ld iy,(**)",   4, 20, Operand::IY, Operand::indNN});
			map_inst.emplace(0xfd2b, Instruction{InstType::DEC,  "dec iy",       2, 10, Operand::IY, Operand::ONE});
			map_inst.emplace(0xfd34, Instruction{InstType::INC,  "inc (iy+*)",   3, 23, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfd35, Instruction{InstType::DEC,  "dec (iy+*)",   3, 23, Operand::indIYN, Operand::ONE});
			map_inst.emplace(0xfd36, Instruction{InstType::LD,   "ld (iy+*),*",  4, 19, Operand::indIYN, Operand::N});
			map_inst.emplace(0xfd39, Instruction{InstType::ADD,  "add iy,sp",    2, 15, Operand::IY, Operand::SP});
			map_inst.emplace(0xfd46, Instruction{InstType::LD,   "ld b,(iy+*)",  3, 19, Operand::B, Operand::indIYN});
			map_inst.emplace(0xfd4e, Instruction{InstType::LD,   "ld c,(iy+*)",  3, 19, Operand::C, Operand::indIYN});
			map_inst.emplace(0xfd56, Instruction{InstType::LD,   "ld d,(iy+*)",  3, 19, Operand::D, Operand::indIYN});
			map_inst.emplace(0xfd5e, Instruction{InstType::LD,   "ld e,(iy+*)",  3, 19, Operand::E, Operand::indIYN});
			map_inst.emplace(0xfd66, Instruction{InstType::LD,   "ld h,(iy+*)",  3, 19, Operand::H, Operand::indIYN});
			map_inst.emplace(0xfd6e, Instruction{InstType::LD,   "ld j,(iy+*)",  3, 19, Operand::L, Operand::indIYN});
			map_inst.emplace(0xfd70, Instruction{InstType::LD,   "ld (iy+*),b",  3, 19, Operand::indIYN, Operand::B});
			map_inst.emplace(0xfd71, Instruction{InstType::LD,   "ld (iy+*),c",  3, 19, Operand::indIYN, Operand::C});
			map_inst.emplace(0xfd72, Instruction{InstType::LD,   "ld (iy+*),d",  3, 19, Operand::indIYN, Operand::D});
			map_inst.emplace(0xfd73, Instruction{InstType::LD,   "ld (iy+*),e",  3, 19, Operand::indIYN, Operand::E});
			map_inst.emplace(0xfd74, Instruction{InstType::LD,   "ld (iy+*),h",  3, 19, Operand::indIYN, Operand::H});
			map_inst.emplace(0xfd75, Instruction{InstType::LD,   "ld (iy+*),l",  3, 19, Operand::indIYN, Operand::L});
			map_inst.emplace(0xfd77, Instruction{InstType::LD,   "ld (iy+*),a",  3, 19, Operand::indIYN, Operand::A});
			map_inst.emplace(0xfd7e, Instruction{InstType::LD,   "ld a,(iy+*)",  3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfd86, Instruction{InstType::ADD,  "add a,(iy+*)", 3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfd96, Instruction{InstType::SUB,  "sub (iy+*)",   3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfda6, Instruction{InstType::AND,  "and (iy+*)",   3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfdae, Instruction{InstType::XOR,  "xor (iy+*)",   3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfdb6, Instruction{InstType::OR,   "or (iy+*)",    3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfdbe, Instruction{InstType::CP,   "cp (iy+*)",    3, 19, Operand::A, Operand::indIYN});
			map_inst.emplace(0xfde1, Instruction{InstType::POP,  "pop iy",       2, 14, Operand::IX});
			map_inst.emplace(0xfde5, Instruction{InstType::PUSH, "push iy",      2, 15, Operand::UNUSED, Operand::IY});
			map_inst.emplace(0xfde9, Instruction{InstType::JP,   "jp (iy)",      2,  8, Conditional::ALWAYS, Operand::PC, Operand::IY});
			map_inst.emplace(0xfdf9, Instruction{InstType::LD,   "ld sp,iy",     2, 10, Operand::SP, Operand::IY});

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

			// TODO - this needs to be included as a file rather than hardcode. It's a list of ROM routines for the 48.rom
			map_rom.emplace(0x0000, "START");
			map_rom.emplace(0x0008, "ERROR-1");
			map_rom.emplace(0x0010, "PRINT-A-1");
			map_rom.emplace(0x0018, "GET-CHAR");
			map_rom.emplace(0x001c, "TEST-CHAR");
			map_rom.emplace(0x0020, "NEXT-CHAR");
			map_rom.emplace(0x0028, "FP-CALC");
			map_rom.emplace(0x0030, "BC-SPACES");
			map_rom.emplace(0x0038, "MASK-INT");
			map_rom.emplace(0x0048, "KEY-INT");
			map_rom.emplace(0x0053, "ERROR-2");
			map_rom.emplace(0x0055, "ERROR-3");
			map_rom.emplace(0x0066, "RESET");
			map_rom.emplace(0x0070, "NO-RESET");
			map_rom.emplace(0x0074, "CH-ADD+1");
			map_rom.emplace(0x0077, "TEMP-PTR1");
			map_rom.emplace(0x0078, "TEMP-PTR2");
			map_rom.emplace(0x007d, "SKIP-OVER");
			map_rom.emplace(0x0090, "SKIPS");
			map_rom.emplace(0x0283, "KEY-SCAN");
			map_rom.emplace(0x0296, "KEY-LINE");
			map_rom.emplace(0x029f, "KEY-3KEYS");
			map_rom.emplace(0x02a1, "KEY-BITS");
			map_rom.emplace(0x02ab, "KEY-DONE");
			map_rom.emplace(0x02bf, "KEYBOARD");
			map_rom.emplace(0x02c6, "K-ST-LOOP");
			map_rom.emplace(0x02d1, "K-CH-SET");
			map_rom.emplace(0x02f1, "K-NEW");
			map_rom.emplace(0x0308, "K-END");
			map_rom.emplace(0x0310, "K-REPEAT");
			map_rom.emplace(0x031e, "K-TEST");
			map_rom.emplace(0x032c, "K-MAIN");
			map_rom.emplace(0x0333, "K-DECODE");
			map_rom.emplace(0x0341, "K-E-LET");
			map_rom.emplace(0x034a, "K-LOOK-UP");
			map_rom.emplace(0x034f, "K-KLC-LET");
			map_rom.emplace(0x0364, "K-TOKENS");
			map_rom.emplace(0x0367, "K-DIGIT");
			map_rom.emplace(0x0382, "K-8-&-9");
			map_rom.emplace(0x0389, "K-GRA-DGT");
			map_rom.emplace(0x039d, "K-KLC-DGT");
			map_rom.emplace(0x03b2, "K-@-CHAR");
			map_rom.emplace(0x03b5, "BEEPER");
			map_rom.emplace(0x03d1, "BE-IX+3");
			map_rom.emplace(0x03d2, "BE-IX+2");
			map_rom.emplace(0x03d3, "BE-IX+1");
			map_rom.emplace(0x03d4, "BE-IX+0");
			map_rom.emplace(0x03d6, "BE-H&L-LP");
			map_rom.emplace(0x03f2, "BE-AGAIN");
			map_rom.emplace(0x03f6, "BE-END");
			map_rom.emplace(0x03f8, "BEEP");
			map_rom.emplace(0x0425, "BE-I-OK");
			map_rom.emplace(0x0427, "BE-OCTAVE");
			map_rom.emplace(0x046c, "REPORT-B");
			map_rom.emplace(0x04c2, "SA-BYTES");
			map_rom.emplace(0x04d0, "SA-FLAG");
			map_rom.emplace(0x04d8, "SA-LEADER");
			map_rom.emplace(0x04ea, "SA-SYNC-1");
			map_rom.emplace(0x04f2, "SA-SYNC-2");
			map_rom.emplace(0x04fe, "SA-LOOP");
			map_rom.emplace(0x0505, "SA-LOOP-P");
			map_rom.emplace(0x0507, "SA-START");
			map_rom.emplace(0x050e, "SA-PARITY");
			map_rom.emplace(0x0511, "SA-BIT-2");
			map_rom.emplace(0x0514, "SA-BIT-1");
			map_rom.emplace(0x051a, "SA-SET");
			map_rom.emplace(0x051c, "SA-OUT");
			map_rom.emplace(0x0525, "SA-8-BITS");
			map_rom.emplace(0x053c, "SA-DELAY");
			map_rom.emplace(0x053f, "SA/LD-RET");
			map_rom.emplace(0x0552, "REPORT-D");
			map_rom.emplace(0x0554, "SA/LD-END");
			map_rom.emplace(0x0556, "LD-BYTES");
			map_rom.emplace(0x056b, "LD-BREAK");
			map_rom.emplace(0x056c, "LD-START");
			map_rom.emplace(0x0574, "LD-WAIT");
			map_rom.emplace(0x0580, "LD-LEADER");
			map_rom.emplace(0x058f, "LD-SYNC");
			map_rom.emplace(0x05a9, "LD-LOOP");
			map_rom.emplace(0x05b3, "LD-FLAG");
			map_rom.emplace(0x05bd, "LD-VERIFY");
			map_rom.emplace(0x05c2, "LD-NEXT");
			map_rom.emplace(0x05c4, "LD-DEC");
			map_rom.emplace(0x05c8, "LD-MARKER");
			map_rom.emplace(0x05ca, "LD-8-BITS");
			map_rom.emplace(0x05e3, "LD-EDGE-2");
			map_rom.emplace(0x05e7, "LD-EDGE-1");
			map_rom.emplace(0x05e9, "LD-DELAY");
			map_rom.emplace(0x05ed, "LD-SAMPLE");
			map_rom.emplace(0x0605, "SAVE-ETC");
			map_rom.emplace(0x0621, "SA-SPACE");
			map_rom.emplace(0x0629, "SA-BLANK");
			map_rom.emplace(0x0642, "REPORT-F");
			map_rom.emplace(0x0644, "SA-NULL");
			map_rom.emplace(0x064b, "SA-NAME");
			map_rom.emplace(0x0652, "SA-DATA");
			map_rom.emplace(0x0670, "REPORT-2");
			map_rom.emplace(0x0672, "SA-V-OLD");
			map_rom.emplace(0x0685, "SA-V-NEW");
			map_rom.emplace(0x068f, "SA-V-TYPE");
			map_rom.emplace(0x0692, "SA-DATA-1");
			map_rom.emplace(0x06a0, "SA-SCR$");
			map_rom.emplace(0x06c3, "SA-CODE");
			map_rom.emplace(0x06e1, "SA-CODE-1");
			map_rom.emplace(0x06f0, "SA-CODE-2");
			map_rom.emplace(0x06f5, "SA-CODE-3");
			map_rom.emplace(0x06f9, "SA-CODE-4");
			map_rom.emplace(0x0710, "SA-TYPE-3");
			map_rom.emplace(0x0716, "SA-LINE");
			map_rom.emplace(0x0723, "SA-LINE-1");
			map_rom.emplace(0x073a, "SA-TYPE-0");
			map_rom.emplace(0x075a, "SA-ALL");
			map_rom.emplace(0x0767, "LD-LOOK-H");
			map_rom.emplace(0x078a, "LD-TYPE");
			map_rom.emplace(0x07ad, "LD-CH-PR");
			map_rom.emplace(0x07cb, "VR-CONTRL");
			map_rom.emplace(0x07e9, "VR-CONT-1");
			map_rom.emplace(0x07f4, "VR-CONT-2");
			map_rom.emplace(0x0800, "VR-CONT-3");
			map_rom.emplace(0x0802, "LD-BLOCK");
			map_rom.emplace(0x0806, "REPORT-R");
			map_rom.emplace(0x0808, "LD-CONTRL");
			map_rom.emplace(0x0819, "LD-CONT-1");
			map_rom.emplace(0x0825, "LD-CONT-2");
			map_rom.emplace(0x082e, "LD-DATA");
			map_rom.emplace(0x084c, "LD-DATA-1");
			map_rom.emplace(0x0873, "LD-PROG");
			map_rom.emplace(0x08ad, "LD-PROG-1");
			map_rom.emplace(0x08b6, "ME-CONTRL");
			map_rom.emplace(0x08d2, "ME-NEW-LP");
			map_rom.emplace(0x08d7, "ME-OLD-LP");
			map_rom.emplace(0x08df, "ME-OLD-L1");
			map_rom.emplace(0x08eb, "ME-NEW-L2");
			map_rom.emplace(0x08f0, "ME-VAR-LP");
			map_rom.emplace(0x08f9, "ME-OLD-VP");
			map_rom.emplace(0x0901, "ME-OLD-V1");
			map_rom.emplace(0x0909, "ME-OLD-V2");
			map_rom.emplace(0x0912, "ME-OLD-V3");
			map_rom.emplace(0x091e, "ME-OLD-V4");
			map_rom.emplace(0x0921, "ME-VAR-L1");
			map_rom.emplace(0x0923, "ME-VAR-L2");
			map_rom.emplace(0x092c, "ME-ENTER");
			map_rom.emplace(0x093e, "ME-ENT-1");
			map_rom.emplace(0x0955, "ME-ENT-2");
			map_rom.emplace(0x0958, "ME-ENT-3");
			map_rom.emplace(0x0970, "SA-CONTROL");
			map_rom.emplace(0x0991, "SA-1-SEC");
			map_rom.emplace(0x09f4, "PRINT-OUT");
			map_rom.emplace(0x0a23, "PO-BACK-1");
			map_rom.emplace(0x0a38, "PO-BACK-2");
			map_rom.emplace(0x0a3a, "PO-BACK-3");
			map_rom.emplace(0x0a3d, "PO-RIGHT");
			map_rom.emplace(0x0a4f, "PO-ENTER");
			map_rom.emplace(0x0a5f, "PO-COMMA");
			map_rom.emplace(0x0a69, "PO-QUEST");
			map_rom.emplace(0x0a6d, "PO-TV-2");
			map_rom.emplace(0x0a75, "PO-2-OPER");
			map_rom.emplace(0x0a7a, "PO-1-OPER");
			map_rom.emplace(0x0a7d, "PO-TV-1");
			map_rom.emplace(0x0a80, "PO-CHANGE");
			map_rom.emplace(0x0a87, "PO-CONT");
			map_rom.emplace(0x0aac, "PO-AT-ERR");
			map_rom.emplace(0x0abf, "PO-AT-SET");
			map_rom.emplace(0x0ac2, "PO-TAB");
			map_rom.emplace(0x0ac3, "PO-FILL");
			map_rom.emplace(0x0ad0, "PO-SPACE");
			map_rom.emplace(0x0ad9, "PO-ABLE");
			map_rom.emplace(0x0adc, "PO-STORE");
			map_rom.emplace(0x0af0, "PO-ST-E");
			map_rom.emplace(0x0afc, "PO-ST-PR");
			map_rom.emplace(0x0b03, "PO-FETCH");
			map_rom.emplace(0x0b1d, "PO-F-PR");
			map_rom.emplace(0x0b24, "PO-ANY");
			map_rom.emplace(0x0b38, "PO-GR-1");
			map_rom.emplace(0x0b3e, "PO-GR-2");
			map_rom.emplace(0x0b4c, "PO-GR-3");
			map_rom.emplace(0x0b52, "PO-T&UDG");
			map_rom.emplace(0x0b5f, "PO-T");
			map_rom.emplace(0x0b65, "PO-CHAR");
			map_rom.emplace(0x0b6a, "PO-CHAR-2");
			map_rom.emplace(0x0b76, "PO-CHAR-3");
			map_rom.emplace(0x0b7f, "PR-ALL");
			map_rom.emplace(0x0b93, "PR-ALL-1");
			map_rom.emplace(0x0ba4, "PR-ALL-2");
			map_rom.emplace(0x0bb6, "PR-ALL-3");
			map_rom.emplace(0x0b97, "PR-ALL-4");
			map_rom.emplace(0x0bc1, "PR-ALL-5");
			map_rom.emplace(0x0bd3, "PR-ALL-6");
			map_rom.emplace(0x0bdb, "PO-ATTR");
			map_rom.emplace(0x0bfa, "PO-ATTR-1");
			map_rom.emplace(0x0c08, "PO-ATTR-2");
			map_rom.emplace(0x0c0a, "PO-MSG");
			map_rom.emplace(0x0c10, "PO-TOKENS");
			map_rom.emplace(0x0c14, "PO-TABLE");
			map_rom.emplace(0x0c22, "PO-EACH");
			map_rom.emplace(0x0c35, "PO-TR-SP");
			map_rom.emplace(0x0c3b, "PO-SAVE");
			map_rom.emplace(0x0c41, "PO-SEARCH");
			map_rom.emplace(0x0c44, "PO-STEP");
			map_rom.emplace(0x0c55, "PO-SCR");
			map_rom.emplace(0x0c86, "REPORT-5");
			map_rom.emplace(0x0c88, "PO-SCR-2");
			map_rom.emplace(0x0cd2, "PO-SCR-3");
			map_rom.emplace(0x0cf0, "PO-SCR-3A");
			map_rom.emplace(0x0d00, "REPORT-D");
			map_rom.emplace(0x0d02, "PO-SCR-4");
			map_rom.emplace(0x0d1c, "PO-SCR-4A");
			map_rom.emplace(0x0d2d, "PO-SCR-4B");
			map_rom.emplace(0x0d4d, "TEMPS");
			map_rom.emplace(0x0d5b, "TEMPS-1");
			map_rom.emplace(0x0d65, "TEMPS-2");
			map_rom.emplace(0x0d6b, "CLS");
			map_rom.emplace(0x0d6e, "CLS-LOWER");
			map_rom.emplace(0x0d87, "CLS-1");
			map_rom.emplace(0x0d89, "CLS-2");
			map_rom.emplace(0x0d8e, "CLS-3");
			map_rom.emplace(0x0d94, "CL-CHAN");
			map_rom.emplace(0x0da0, "CL-CHAN-A");
			map_rom.emplace(0x0daf, "CL-ALL");
			map_rom.emplace(0x0dd9, "CL-SET");
			map_rom.emplace(0x0dee, "CL-SET-1");
			map_rom.emplace(0x0df4, "CL-SET-2");
			map_rom.emplace(0x0dfe, "CL-SC-ALL");
			map_rom.emplace(0x0e00, "CL-SCROLL");
			map_rom.emplace(0x0e05, "CL-SCR-1");
			map_rom.emplace(0x0e0d, "CL-SCR-2");
			map_rom.emplace(0x0e19, "CL-SCR-3");
			map_rom.emplace(0x0e44, "CL-LINE");
			map_rom.emplace(0x0e4a, "CL-LINE-1");
			map_rom.emplace(0x0e4d, "CL-LINE-2");
			map_rom.emplace(0x0e80, "CL-LINE-3");
			map_rom.emplace(0x0e88, "CL-ATTR");
			map_rom.emplace(0x0e9b, "CL-ADDR");
			map_rom.emplace(0x0eac, "COPY");
			map_rom.emplace(0x0eb2, "COPY-1");
			map_rom.emplace(0x0ec9, "COPY-2");
			map_rom.emplace(0x0ecd, "COPY-BUFF");
			map_rom.emplace(0x0ed3, "COPY-3");
			map_rom.emplace(0x0eda, "COPY-END");
			map_rom.emplace(0x0edf, "CLEAR-PRB");
			map_rom.emplace(0x0ee7, "PRB-BYTES");
			map_rom.emplace(0x0ef4, "COPY-LINE");
			map_rom.emplace(0x0efd, "COPY-L-1");
			map_rom.emplace(0x0f0c, "COPY-L-2");
			map_rom.emplace(0x0f14, "COPY-L-3");
			map_rom.emplace(0x0f18, "COPY-L-4");
			map_rom.emplace(0x0f1e, "COPY-L-5");
			map_rom.emplace(0x0f2c, "EDITOR");
			map_rom.emplace(0x0f30, "ED-AGAIN");
			map_rom.emplace(0x0f38, "ED-LOOP");
			map_rom.emplace(0x0f6c, "ED-CONTR");
			map_rom.emplace(0x0f81, "ADD-CHAR");
			map_rom.emplace(0x0f8b, "ADD-CH-1");
			map_rom.emplace(0x0f92, "ED-KEYS");
			map_rom.emplace(0x0fa9, "ED-EDIT");
			map_rom.emplace(0x0ff3, "ED-DOWN");
			map_rom.emplace(0x1001, "ED-STOP");
			map_rom.emplace(0x1007, "ED-LEFT");
			map_rom.emplace(0x100c, "ED-RIGHT");
			map_rom.emplace(0x1011, "ED-CUR");
			map_rom.emplace(0x1015, "ED-DELETE");
			map_rom.emplace(0x101e, "ED-IGNORE");
			map_rom.emplace(0x1024, "ED-ENTER");
			map_rom.emplace(0x1026, "ED-END");
			map_rom.emplace(0x1031, "ED-EDGE");
			map_rom.emplace(0x103e, "ED-EDGE-1");
			map_rom.emplace(0x1051, "ED-EDGE-2");
			map_rom.emplace(0x1059, "ED-UP");
			map_rom.emplace(0x106e, "ED-LIST");
			map_rom.emplace(0x1076, "ED-SYMBOL");
			map_rom.emplace(0x107c, "ED-GRAPH");
			map_rom.emplace(0x107f, "ED-ERROR");
			map_rom.emplace(0x1097, "CLEAR-SP");
			map_rom.emplace(0x10a8, "KEY-INPUT");
			map_rom.emplace(0x10db, "KEY-M&CL");
			map_rom.emplace(0x10e6, "KEY-MODE");
			map_rom.emplace(0x10f4, "KEY-FLAG");
			map_rom.emplace(0x10fa, "KEY-CONTR");
			map_rom.emplace(0x1105, "KEY-DATA");
			map_rom.emplace(0x110d, "KEY-NEXT");
			map_rom.emplace(0x1113, "KEY-CHAN");
			map_rom.emplace(0x111b, "KEY-DONE");
			map_rom.emplace(0x111d, "ED-COPY");
			map_rom.emplace(0x1150, "ED-BLANK");
			map_rom.emplace(0x115e, "ED-SPACES");
			map_rom.emplace(0x1167, "ED-FULL");
			map_rom.emplace(0x117c, "ED-C-DONE");
			map_rom.emplace(0x117e, "ED-C-END");
			map_rom.emplace(0x1190, "SET-HL");
			map_rom.emplace(0x1195, "SET-DE");
			map_rom.emplace(0x11a7, "REMOVE-FP");
			map_rom.emplace(0x11b7, "NEW");
			map_rom.emplace(0x11cb, "START/NEW");
			map_rom.emplace(0x11da, "RAM-CHECK");
			map_rom.emplace(0x11dc, "RAM-FILL");
			map_rom.emplace(0x11e2, "RAM-READ");
			map_rom.emplace(0x11ef, "RAM-DONE");
			map_rom.emplace(0x1219, "RAM-SET");
			map_rom.emplace(0x12a2, "MAIN-EXEC");
			map_rom.emplace(0x12a9, "MAIN-1");
			map_rom.emplace(0x12ac, "MAIN-2");
			map_rom.emplace(0x12cf, "MAIN-3");
			map_rom.emplace(0x1303, "MAIN-4");
			map_rom.emplace(0x1313, "MAIN-G");
			map_rom.emplace(0x133c, "MAIN-5");
			map_rom.emplace(0x1373, "MAIN-6");
			map_rom.emplace(0x1376, "MAIN-7");
			map_rom.emplace(0x1384, "MAIN-8");
			map_rom.emplace(0x1386, "MAIN-9");
		}

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

	bool clock()
		{
			bool found = false;

			const auto opcode = get_opcode(pc.get());
			std::cout << dump_instr_at_pc(pc.get()).str() << std::endl;
			auto search = map_inst.find(opcode);
			if (search != map_inst.end())
			{
				Instruction &inst = search->second;
				pc.set(curr_opcode_pc + inst.size);
				inst.execute(*this);
				found = true;
			}

			return found;
		}
	std::stringstream dump_instr_at_pc(unsigned short pc)
		{
			std::stringstream str;

			const auto opcode = get_opcode(pc);
			auto search_op = map_inst.find(opcode);
			if (search_op != map_inst.end())
			{
				Instruction &inst = search_op->second;
				str << std::left << std::setw(20) << mem.dump(curr_opcode_pc, inst.size);
				str << std::setw(20) << inst.name;

				auto search_rom = map_rom.find(pc);
				if (search_rom != map_rom.end())
				{
					str << "Routine: " << search_rom->second;
				}
			}
			else
			{
				str << mem.dump(curr_opcode_pc, 4) << " UNKNOWN INSTRUCTION: 0x" << std::hex << opcode << std::dec;
			}
			return str;
		}
	void dump()
		{
			std::cout << "AF: " << af << std::endl;
			std::cout << "HL: " << hl << std::endl;
			std::cout << "BC: " << bc << std::endl;
			std::cout << "DE: " << de << std::endl;
			std::cout << "IX: " << ix << std::endl;
			std::cout << "IY: " << iy << std::endl;
			std::cout << "PC: " << pc << std::endl;
			std::cout << "SP: " << sp << std::endl;
		}
	void dump_sp()
		{
			assert(sp.get() <= top_of_stack);
			std::cout << "Dumping stack at SP: " << sp << std::endl;
			std::cout << mem.dump(sp.get(), top_of_stack - sp.get()) << std::endl;
			std::cout << "==== TOP OF THE STACK ====" << std::endl;
		}
	void reset()
		{
			pc.reset();
			af.reset();
			sp.reset();

			af.set(0xffff);
			sp.set(0xffff);

			top_of_stack = 0xffff;
		}

private:
	unsigned int get_opcode(unsigned short pc)
		{
			curr_opcode_pc = pc;
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
	std::map<unsigned int, std::string> map_rom;
};

#endif // __Z80_HPP__

