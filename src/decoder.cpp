#include <map>
#include <iomanip>

#include "decoder.hpp"
#include "common.hpp"

static std::map<uint32_t, Instruction> map_inst;
static std::map<uint32_t, std::string> map_rom;

static Instruction inv_inst{InstType::INV, "INVALID", 0, 0};
static std::string unk_rom_addr{""};

void init_map_inst()
{
	map_inst.emplace(0x00, Instruction{InstType::NOP,  "nop",        1,  4});
	map_inst.emplace(0x01, Instruction{InstType::LD,   "ld bc,**",   3, 10, Operand::BC, Operand::NN});
	map_inst.emplace(0x02, Instruction{InstType::LD,   "ld (bc),a",  1,  7, Operand::indBC, Operand::A});
	map_inst.emplace(0x03, Instruction{InstType::INC,  "inc bc",     1,  6, Operand::BC, Operand::ONE});
	map_inst.emplace(0x04, Instruction{InstType::INC,  "inc b",      1,  4, Operand::B, Operand::ONE});
	map_inst.emplace(0x05, Instruction{InstType::DEC,  "dec b",      1,  4, Operand::B, Operand::ONE});
	map_inst.emplace(0x06, Instruction{InstType::LD,   "ld b,*",     2,  7, Operand::B, Operand::N});
	map_inst.emplace(0x07, Instruction{InstType::RLCA, "rlca",       1,  4, Operand::A});
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
	map_inst.emplace(0x17, Instruction{InstType::RLA,  "rla",        1,  4, Operand::A});
	map_inst.emplace(0x18, Instruction{InstType::JR,   "jr *",       2, 18, Conditional::ALWAYS, Operand::PC, Operand::N});
	map_inst.emplace(0x19, Instruction{InstType::ADD,  "add hl,de",  1,  1, Operand::HL, Operand::DE});
	map_inst.emplace(0x1a, Instruction{InstType::LD,   "ld a,(de)",  1,  7, Operand::A, Operand::indDE});
	map_inst.emplace(0x1b, Instruction{InstType::DEC,  "dec de",     1,  6, Operand::DE, Operand::ONE});
	map_inst.emplace(0x1c, Instruction{InstType::INC,  "inc e",      1,  4, Operand::E, Operand::ONE});
	map_inst.emplace(0x1d, Instruction{InstType::DEC,  "dec e",      1,  4, Operand::E, Operand::ONE});
	map_inst.emplace(0x1e, Instruction{InstType::LD,   "ld e,*",     2,  7, Operand::E, Operand::N});
	map_inst.emplace(0x1f, Instruction{InstType::RRA,  "rra",        1,  4, Operand::A});

	map_inst.emplace(0x20, Instruction{InstType::JR,   "jr nz,*",    2, 12, 7, Conditional::NZ, Operand::PC, Operand::N});
	map_inst.emplace(0x21, Instruction{InstType::LD,   "ld hl,**",   3, 10, Operand::HL, Operand::NN});
	map_inst.emplace(0x22, Instruction{InstType::LD,   "ld (**),hl", 3, 16, Operand::indNN, Operand::HL});
	map_inst.emplace(0x23, Instruction{InstType::INC,  "inc hl",     1,  6, Operand::HL, Operand::ONE});
	map_inst.emplace(0x24, Instruction{InstType::INC,  "inc h",      1,  4, Operand::H, Operand::ONE});
	map_inst.emplace(0x25, Instruction{InstType::DEC,  "dec h",      1,  4, Operand::H, Operand::ONE});
	map_inst.emplace(0x26, Instruction{InstType::LD,   "ld h,*",     2,  7, Operand::H, Operand::N});
	map_inst.emplace(0x27, Instruction{InstType::DAA,  "daa",        1,  4, Operand::A, Operand::A});
	map_inst.emplace(0x28, Instruction{InstType::JR,   "jr z,*",     2, 12, 7, Conditional::Z, Operand::PC, Operand::N});
	map_inst.emplace(0x29, Instruction{InstType::ADD,  "add hl,hl",  1, 11, Operand::HL, Operand::HL});
	map_inst.emplace(0x2a, Instruction{InstType::LD,   "ld hl,(**)", 3, 16, Operand::HL, Operand::indNN});
	map_inst.emplace(0x2b, Instruction{InstType::DEC,  "dec hl",     1,  6, Operand::HL, Operand::ONE});
	map_inst.emplace(0x2c, Instruction{InstType::INC,  "inc l",      1,  4, Operand::L, Operand::ONE});
	map_inst.emplace(0x2d, Instruction{InstType::DEC,  "dec l",      1,  4, Operand::L, Operand::ONE});
	map_inst.emplace(0x2e, Instruction{InstType::LD,   "ld l,*",     2,  7, Operand::L, Operand::N});
	map_inst.emplace(0x2f, Instruction{InstType::CPL,  "cpl",        1,  4});

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
	map_inst.emplace(0x76, Instruction{InstType::HALT, "halt",       1,  4});
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
	map_inst.emplace(0x88, Instruction{InstType::ADC,  "adc a,b",    1,  4, Operand::A, Operand::B});
	map_inst.emplace(0x89, Instruction{InstType::ADC,  "adc a,c",    1,  4, Operand::A, Operand::C});
	map_inst.emplace(0x8a, Instruction{InstType::ADC,  "adc a,d",    1,  4, Operand::A, Operand::D});
	map_inst.emplace(0x8b, Instruction{InstType::ADC,  "adc a,e",    1,  4, Operand::A, Operand::E});
	map_inst.emplace(0x8c, Instruction{InstType::ADC,  "adc a,h",    1,  4, Operand::A, Operand::H});
	map_inst.emplace(0x8d, Instruction{InstType::ADC,  "adc a,l",    1,  4, Operand::A, Operand::L});
	map_inst.emplace(0x8e, Instruction{InstType::ADC,  "adc a,(hl)", 1,  7, Operand::A, Operand::indHL});
	map_inst.emplace(0x8f, Instruction{InstType::ADC,  "adc a,a",    1,  4, Operand::A, Operand::A});

	map_inst.emplace(0x90, Instruction{InstType::SUB,  "sub b",      1,  4, Operand::A, Operand::B});
	map_inst.emplace(0x91, Instruction{InstType::SUB,  "sub c",      1,  4, Operand::A, Operand::C});
	map_inst.emplace(0x92, Instruction{InstType::SUB,  "sub d",      1,  4, Operand::A, Operand::D});
	map_inst.emplace(0x93, Instruction{InstType::SUB,  "sub e",      1,  4, Operand::A, Operand::E});
	map_inst.emplace(0x94, Instruction{InstType::SUB,  "sub h",      1,  4, Operand::A, Operand::H});
	map_inst.emplace(0x95, Instruction{InstType::SUB,  "sub l",      1,  4, Operand::A, Operand::L});
	map_inst.emplace(0x96, Instruction{InstType::SUB,  "sub (hl)",   1,  7, Operand::A, Operand::indHL});
	map_inst.emplace(0x97, Instruction{InstType::SUB,  "sub a",      1,  4, Operand::A, Operand::A});
	map_inst.emplace(0x98, Instruction{InstType::SBC,  "sbc a,b",    1,  4, Operand::A, Operand::B});
	map_inst.emplace(0x99, Instruction{InstType::SBC,  "sbc a,c",    1,  4, Operand::A, Operand::C});
	map_inst.emplace(0x9a, Instruction{InstType::SBC,  "sbc a,d",    1,  4, Operand::A, Operand::D});
	map_inst.emplace(0x9b, Instruction{InstType::SBC,  "sbc a,e",    1,  4, Operand::A, Operand::E});
	map_inst.emplace(0x9c, Instruction{InstType::SBC,  "sbc a,h",    1,  4, Operand::A, Operand::H});
	map_inst.emplace(0x9d, Instruction{InstType::SBC,  "sbc a,l",    1,  4, Operand::A, Operand::L});
	map_inst.emplace(0x9e, Instruction{InstType::SBC,  "sbc a,(hl)", 1,  7, Operand::A, Operand::indHL});
	map_inst.emplace(0x9f, Instruction{InstType::SBC,  "sbc a,a",    1,  4, Operand::A, Operand::A});

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
	// 0xCD (bits) instructions
	map_inst.emplace(0xcc, Instruction{InstType::CALL, "call z,**",  3, 17, 10, Conditional::Z, Operand::PC, Operand::NN});
	map_inst.emplace(0xcd, Instruction{InstType::CALL, "call **",    3, 17, Conditional::ALWAYS, Operand::PC, Operand::NN});
	map_inst.emplace(0xce, Instruction{InstType::ADC,  "adc a,*",    2,  7, Operand::A, Operand::N});
	map_inst.emplace(0xcf, Instruction{InstType::RST,  "rst 08h",    1, 11, Operand::PC, Operand::HEX_0008});

	map_inst.emplace(0xd0, Instruction{InstType::RET,  "ret nc",     1, 11, 5, Conditional::NC, Operand::PC});
	map_inst.emplace(0xd1, Instruction{InstType::POP,  "pop de",     1, 10, Operand::DE});
	map_inst.emplace(0xd2, Instruction{InstType::JP,   "jp nc,**",   3, 10, Conditional::NC, Operand::PC, Operand::NN});
	map_inst.emplace(0xd3, Instruction{InstType::OUT,  "out (*),a",  2, 11, Operand::PORTN, Operand::A});
	map_inst.emplace(0xd4, Instruction{InstType::CALL, "call nc,**", 3, 17, 10, Conditional::NC, Operand::PC, Operand::NN});
	map_inst.emplace(0xd5, Instruction{InstType::PUSH, "push de",    1, 11, Operand::UNUSED, Operand::DE});
	map_inst.emplace(0xd6, Instruction{InstType::SUB,  "sub *",      2,  7, Operand::A, Operand::N});
	map_inst.emplace(0xd7, Instruction{InstType::RST,  "rst 10h",    1, 11, Operand::PC, Operand::HEX_0010});
	map_inst.emplace(0xd8, Instruction{InstType::RET,  "ret c",      1, 11, 5, Conditional::C, Operand::PC});
	map_inst.emplace(0xd9, Instruction{InstType::EX,   "exx",        1,  4});
	map_inst.emplace(0xda, Instruction{InstType::JP,   "jp c,**",    3, 10, Conditional::C, Operand::PC, Operand::NN});
	map_inst.emplace(0xdb, Instruction{InstType::IN,   "in a,(*)",   2, 11, Operand::A, Operand::PORTN});
	map_inst.emplace(0xdc, Instruction{InstType::CALL, "call c,**",  3, 17, 10, Conditional::C, Operand::PC, Operand::NN});
	// 0xDB (ix) instructions
	map_inst.emplace(0xde, Instruction{InstType::SBC,  "sbc a,*",    2,  7, Operand::A, Operand::N});
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
	// 0xED (extd) instructions
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
	// 0xFD (iy) instructions
	map_inst.emplace(0xfe, Instruction{InstType::CP,   "cp *",       2,  7, Operand::A, Operand::N});
	map_inst.emplace(0xff, Instruction{InstType::RST,  "rst 38h",    1, 11, Operand::PC, Operand::HEX_0038});



	map_inst.emplace(0xcb00, Instruction{InstType::RLC,  "rlc b",    2,  8, Operand::B});
	map_inst.emplace(0xcb01, Instruction{InstType::RLC,  "rlc c",    2,  8, Operand::C});
	map_inst.emplace(0xcb02, Instruction{InstType::RLC,  "rlc d",    2,  8, Operand::D});
	map_inst.emplace(0xcb03, Instruction{InstType::RLC,  "rlc e",    2,  8, Operand::E});
	map_inst.emplace(0xcb04, Instruction{InstType::RLC,  "rlc h",    2,  8, Operand::H});
	map_inst.emplace(0xcb05, Instruction{InstType::RLC,  "rlc l",    2,  8, Operand::L});
	map_inst.emplace(0xcb06, Instruction{InstType::RLC,  "rlc (hl)", 2, 15, Operand::indHL});
	map_inst.emplace(0xcb07, Instruction{InstType::RLC,  "rlc a",    2,  8, Operand::A});
	map_inst.emplace(0xcb08, Instruction{InstType::RRC,  "rrc b",    2,  8, Operand::B});
	map_inst.emplace(0xcb09, Instruction{InstType::RRC,  "rrc c",    2,  8, Operand::C});
	map_inst.emplace(0xcb0a, Instruction{InstType::RRC,  "rrc d",    2,  8, Operand::D});
	map_inst.emplace(0xcb0b, Instruction{InstType::RRC,  "rrc e",    2,  8, Operand::E});
	map_inst.emplace(0xcb0c, Instruction{InstType::RRC,  "rrc h",    2,  8, Operand::H});
	map_inst.emplace(0xcb0d, Instruction{InstType::RRC,  "rrc l",    2,  8, Operand::L});
	map_inst.emplace(0xcb0e, Instruction{InstType::RRC,  "rrc (hl)", 2, 15, Operand::indHL});
	map_inst.emplace(0xcb0f, Instruction{InstType::RRC,  "rrc a",    2,  8, Operand::A});

	map_inst.emplace(0xcb10, Instruction{InstType::RL,   "rl b",     2,  8, Operand::B});
	map_inst.emplace(0xcb11, Instruction{InstType::RL,   "rl c",     2,  8, Operand::C});
	map_inst.emplace(0xcb12, Instruction{InstType::RL,   "rl d",     2,  8, Operand::D});
	map_inst.emplace(0xcb13, Instruction{InstType::RL,   "rl e",     2,  8, Operand::E});
	map_inst.emplace(0xcb14, Instruction{InstType::RL,   "rl h",     2,  8, Operand::H});
	map_inst.emplace(0xcb15, Instruction{InstType::RL,   "rl l",     2,  8, Operand::L});
	map_inst.emplace(0xcb16, Instruction{InstType::RL,   "rl (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb17, Instruction{InstType::RL,   "rl a",     2,  8, Operand::A});
	map_inst.emplace(0xcb18, Instruction{InstType::RR,   "rr b",     2,  8, Operand::B});
	map_inst.emplace(0xcb19, Instruction{InstType::RR,   "rr c",     2,  8, Operand::C});
	map_inst.emplace(0xcb1a, Instruction{InstType::RR,   "rr d",     2,  8, Operand::D});
	map_inst.emplace(0xcb1b, Instruction{InstType::RR,   "rr e",     2,  8, Operand::E});
	map_inst.emplace(0xcb1c, Instruction{InstType::RR,   "rr h",     2,  8, Operand::H});
	map_inst.emplace(0xcb1d, Instruction{InstType::RR,   "rr l",     2,  8, Operand::L});
	map_inst.emplace(0xcb1e, Instruction{InstType::RR,   "rr (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb1f, Instruction{InstType::RR,   "rr a",     2,  8, Operand::A});

	map_inst.emplace(0xcb20, Instruction{InstType::SLA,  "sla b",     2,  8, Operand::B});
	map_inst.emplace(0xcb21, Instruction{InstType::SLA,  "sla c",     2,  8, Operand::C});
	map_inst.emplace(0xcb22, Instruction{InstType::SLA,  "sla d",     2,  8, Operand::D});
	map_inst.emplace(0xcb23, Instruction{InstType::SLA,  "sla e",     2,  8, Operand::E});
	map_inst.emplace(0xcb24, Instruction{InstType::SLA,  "sla h",     2,  8, Operand::H});
	map_inst.emplace(0xcb25, Instruction{InstType::SLA,  "sla l",     2,  8, Operand::L});
	map_inst.emplace(0xcb26, Instruction{InstType::SLA,  "sla (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb27, Instruction{InstType::SLA,  "sla a",     2,  8, Operand::A});
	map_inst.emplace(0xcb28, Instruction{InstType::SRA,  "sra b",     2,  8, Operand::B});
	map_inst.emplace(0xcb29, Instruction{InstType::SRA,  "sra c",     2,  8, Operand::C});
	map_inst.emplace(0xcb2a, Instruction{InstType::SRA,  "sra d",     2,  8, Operand::D});
	map_inst.emplace(0xcb2b, Instruction{InstType::SRA,  "sra e",     2,  8, Operand::E});
	map_inst.emplace(0xcb2c, Instruction{InstType::SRA,  "sra h",     2,  8, Operand::H});
	map_inst.emplace(0xcb2d, Instruction{InstType::SRA,  "sra l",     2,  8, Operand::L});
	map_inst.emplace(0xcb2e, Instruction{InstType::SRA,  "sra (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb2f, Instruction{InstType::SRA,  "sra a",     2,  8, Operand::A});

	map_inst.emplace(0xcb30, Instruction{InstType::SLL,  "sll b",     2,  8, Operand::B});
	map_inst.emplace(0xcb31, Instruction{InstType::SLL,  "sll c",     2,  8, Operand::C});
	map_inst.emplace(0xcb32, Instruction{InstType::SLL,  "sll d",     2,  8, Operand::D});
	map_inst.emplace(0xcb33, Instruction{InstType::SLL,  "sll e",     2,  8, Operand::E});
	map_inst.emplace(0xcb34, Instruction{InstType::SLL,  "sll h",     2,  8, Operand::H});
	map_inst.emplace(0xcb35, Instruction{InstType::SLL,  "sll l",     2,  8, Operand::L});
	map_inst.emplace(0xcb36, Instruction{InstType::SLL,  "sll (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb37, Instruction{InstType::SLL,  "sll a",     2,  8, Operand::A});
	map_inst.emplace(0xcb38, Instruction{InstType::SRL,  "srl b",     2,  8, Operand::B});
	map_inst.emplace(0xcb39, Instruction{InstType::SRL,  "srl c",     2,  8, Operand::C});
	map_inst.emplace(0xcb3a, Instruction{InstType::SRL,  "srl d",     2,  8, Operand::D});
	map_inst.emplace(0xcb3b, Instruction{InstType::SRL,  "srl e",     2,  8, Operand::E});
	map_inst.emplace(0xcb3c, Instruction{InstType::SRL,  "srl h",     2,  8, Operand::H});
	map_inst.emplace(0xcb3d, Instruction{InstType::SRL,  "srl l",     2,  8, Operand::L});
	map_inst.emplace(0xcb3e, Instruction{InstType::SRL,  "srl (hl)",  2, 15, Operand::indHL});
	map_inst.emplace(0xcb3f, Instruction{InstType::SRL,  "srl a",     2,  8, Operand::A});

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



	map_inst.emplace(0xed40, Instruction{InstType::IN,   "in b,(c)",   2, 12, Operand::B, Operand::PORTC});
	map_inst.emplace(0xed41, Instruction{InstType::OUT,  "out (c),b",  2, 12, Operand::PORTC, Operand::B});
	map_inst.emplace(0xed42, Instruction{InstType::SBC,  "sbc hl,bc",  2, 15, Operand::HL, Operand::BC});
	map_inst.emplace(0xed43, Instruction{InstType::LD,   "ld (**),bc", 4, 20, Operand::indNN, Operand::BC});
	map_inst.emplace(0xed44, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed45, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed46, Instruction{InstType::IM,   "im 0",       2,  8, Operand::IM, Operand::ZERO});
	map_inst.emplace(0xed47, Instruction{InstType::LD,   "ld i,a",     2,  9, Operand::I, Operand::A});
	map_inst.emplace(0xed48, Instruction{InstType::IN,   "in c,(c)",   2, 12, Operand::C, Operand::PORTC});
	map_inst.emplace(0xed49, Instruction{InstType::OUT,  "out (c),c",  2, 12, Operand::PORTC, Operand::C});
	map_inst.emplace(0xed4a, Instruction{InstType::ADC,  "adc hl,bc",  2, 15, Operand::HL, Operand::BC});
	map_inst.emplace(0xed4b, Instruction{InstType::LD,   "ld bc,(**)", 4, 20, Operand::BC, Operand::indNN});
	map_inst.emplace(0xed4c, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed4d, Instruction{InstType::RETI, "reti",       2, 14, Operand::PC});
	//! im 0/1
	map_inst.emplace(0xed4f, Instruction{InstType::LD,   "ld r,a",     2,  9, Operand::R, Operand::A});

	map_inst.emplace(0xed50, Instruction{InstType::IN,   "in d,(c)",   2, 12, Operand::D, Operand::PORTC});
	map_inst.emplace(0xed51, Instruction{InstType::OUT,  "out (c),d",  2, 12, Operand::PORTC, Operand::D});
	map_inst.emplace(0xed52, Instruction{InstType::SBC,  "sbc hl,de",  2, 15, Operand::HL, Operand::DE});
	map_inst.emplace(0xed53, Instruction{InstType::LD,   "ld (**),de", 4, 20, Operand::indNN, Operand::DE});
	map_inst.emplace(0xed54, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed55, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed56, Instruction{InstType::IM,   "im 1",       2,  8, Operand::IM, Operand::ONE});
	map_inst.emplace(0xed57, Instruction{InstType::LD,   "ld a,i",     2,  9, Operand::A, Operand::I});
	map_inst.emplace(0xed58, Instruction{InstType::IN,   "in e,(c)",   2, 12, Operand::E, Operand::PORTC});
	map_inst.emplace(0xed59, Instruction{InstType::OUT,  "out (c),e",  2, 12, Operand::PORTC, Operand::E});
	map_inst.emplace(0xed5a, Instruction{InstType::ADC,  "adc hl,de",  2, 15, Operand::HL, Operand::DE});
	map_inst.emplace(0xed5b, Instruction{InstType::LD,   "ld de,(**)", 4, 20, Operand::DE, Operand::indNN});
	map_inst.emplace(0xed5c, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed5d, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed5e, Instruction{InstType::IM,   "im 2",       2,  8, Operand::IM, Operand::TWO});
	map_inst.emplace(0xed5f, Instruction{InstType::LD,   "ld a,r",     2,  9, Operand::A, Operand::R});

	map_inst.emplace(0xed60, Instruction{InstType::IN,   "in h,(c)",   2, 12, Operand::H, Operand::PORTC});
	map_inst.emplace(0xed61, Instruction{InstType::OUT,  "out (c),h",  2, 12, Operand::PORTC, Operand::H});
	map_inst.emplace(0xed62, Instruction{InstType::SBC,  "sbc hl,hl",  2, 15, Operand::HL, Operand::HL});
	//! ld (**),hl
	map_inst.emplace(0xed64, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed65, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed66, Instruction{InstType::IM,   "im 0",       2,  8, Operand::IM, Operand::ZERO});
	//! rrd
	map_inst.emplace(0xed68, Instruction{InstType::IN,   "in l,(c)",   2, 12, Operand::L, Operand::PORTC});
	map_inst.emplace(0xed69, Instruction{InstType::OUT,  "out (c),l",  2, 12, Operand::PORTC, Operand::L});
	map_inst.emplace(0xed6a, Instruction{InstType::ADC,  "adc hl,hl",  2, 15, Operand::HL, Operand::HL});
	map_inst.emplace(0xed6c, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed6d, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	//! im 0/1
	//! rld

	map_inst.emplace(0xed70, Instruction{InstType::IN,   "in (c)",     2, 12, Operand::ZERO, Operand::PORTC});
	map_inst.emplace(0xed71, Instruction{InstType::OUT,  "out (c),0",  2, 12, Operand::PORTC, Operand::ZERO});
	map_inst.emplace(0xed72, Instruction{InstType::SBC,  "sbc hl,sp",  2, 15, Operand::HL, Operand::SP});
	map_inst.emplace(0xed73, Instruction{InstType::LD,   "ld (**),sp", 4, 20, Operand::indNN, Operand::SP});
	map_inst.emplace(0xed74, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed75, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed76, Instruction{InstType::IM,   "im 1",       2,  8, Operand::IM, Operand::ONE});
	map_inst.emplace(0xed78, Instruction{InstType::IN,   "in a,(c)",   2, 12, Operand::A, Operand::PORTC});
	map_inst.emplace(0xed79, Instruction{InstType::OUT,  "out (c),a",  2, 13, Operand::PORTC, Operand::A});
	map_inst.emplace(0xed7a, Instruction{InstType::ADC,  "adc hl,sp",  2, 15, Operand::HL, Operand::SP});
	map_inst.emplace(0xed7b, Instruction{InstType::LD,   "ld sp,(**)", 4, 20, Operand::SP, Operand::indNN});
	map_inst.emplace(0xed7c, Instruction{InstType::SUB,  "neg",        2,  8, Operand::ZERO, Operand::A});
	map_inst.emplace(0xed7d, Instruction{InstType::RETN, "retn",       2, 14, Operand::PC});
	map_inst.emplace(0xed7e, Instruction{InstType::IM,   "im 2",       2,  8, Operand::IM, Operand::TWO});

	map_inst.emplace(0xeda0, Instruction{InstType::LDI,  "ldi",        2, 16, Operand::indDE, Operand::indHL});
	map_inst.emplace(0xeda1, Instruction{InstType::CPI,  "cpi",        2, 16, 16, Operand::UNUSED, Operand::UNUSED});
	//! ini
	//! outi
	map_inst.emplace(0xeda8, Instruction{InstType::LDD,  "ldd",        2, 16, Operand::indDE, Operand::indHL});
	map_inst.emplace(0xeda9, Instruction{InstType::CPD,  "cpd",        2, 16, 16, Operand::UNUSED, Operand::UNUSED});
	//! ind
	//! outd

	map_inst.emplace(0xedb0, Instruction{InstType::LDIR, "ldir",       2, 21, 16, Operand::indDE, Operand::indHL});
	map_inst.emplace(0xedb1, Instruction{InstType::CPIR, "cpir",       2, 21, 16, Operand::UNUSED, Operand::UNUSED});
	//! inir
	//! otir
	map_inst.emplace(0xedb8, Instruction{InstType::LDDR, "lddr",       2, 21, 16, Operand::indDE, Operand::indHL});
	map_inst.emplace(0xedb9, Instruction{InstType::CPDR, "cpdr",       2, 21, 16, Operand::UNUSED, Operand::UNUSED});
	//! indr
	//! otdr



	map_inst.emplace(0xdd09, Instruction{InstType::ADD,  "add ix,bc",    2, 15, Operand::IX, Operand::BC});

	map_inst.emplace(0xdd19, Instruction{InstType::ADD,  "add ix,de",    2, 15, Operand::IX, Operand::DE});

	map_inst.emplace(0xdd21, Instruction{InstType::LD,   "ld ix,**",     4, 14, Operand::IX, Operand::NN});
	map_inst.emplace(0xdd22, Instruction{InstType::LD,   "ld (**),ix",   4, 20, Operand::indNN, Operand::IX});
	map_inst.emplace(0xdd23, Instruction{InstType::INC,  "inc ix",       2, 10, Operand::IX, Operand::ONE});
	//! inc ixh
	//! dec ixh
	map_inst.emplace(0xdd26, Instruction{InstType::LD,   "ld ixh,*",     3, 11, Operand::IXH, Operand::N});
	map_inst.emplace(0xdd29, Instruction{InstType::ADD,  "add ix,ix",    2, 15, Operand::IX, Operand::IX});
	map_inst.emplace(0xdd2a, Instruction{InstType::LD,   "ld ix,(**)",   4, 20, Operand::IX, Operand::indNN});
	map_inst.emplace(0xdd2b, Instruction{InstType::DEC,  "dec ix",       2, 10, Operand::IX, Operand::ONE});
	//! inc ixl
	//! dec ixl
	//! ld ixl,*

	map_inst.emplace(0xdd34, Instruction{InstType::INC,  "inc (ix+*)",   3, 23, Operand::indIXN, Operand::ONE});
	map_inst.emplace(0xdd35, Instruction{InstType::DEC,  "dec (ix+*)",   3, 23, Operand::indIXN, Operand::ONE});
	map_inst.emplace(0xdd36, Instruction{InstType::LD,   "ld (ix+*),*",  4, 19, Operand::indIXN, Operand::N});
	map_inst.emplace(0xdd39, Instruction{InstType::ADD,  "add ix,sp",    2, 15, Operand::IX, Operand::SP});

	//! ld b,ixh
	//! ld b,ixl
	map_inst.emplace(0xdd46, Instruction{InstType::LD,   "ld b,(ix+*)",  3, 19, Operand::B, Operand::indIXN});
	//! ld c,ixh
	//! ld c,ixl
	map_inst.emplace(0xdd4e, Instruction{InstType::LD,   "ld c,(ix+*)",  3, 19, Operand::C, Operand::indIXN});

	//! ld d,ixh
	//! ld d,ixl
	map_inst.emplace(0xdd56, Instruction{InstType::LD,   "ld d,(ix+*)",  3, 19, Operand::D, Operand::indIXN});
	//! ld e,ixh
	//! ld e,ixl
	map_inst.emplace(0xdd5e, Instruction{InstType::LD,   "ld e,(ix+*)",  3, 19, Operand::E, Operand::indIXN});

	//! ld ixh,b
	//! ld ixh,c
	//! ld ixh,d
	//! ld ixh,e
	//! ld ixh,h
	//! ld ixh,l
	map_inst.emplace(0xdd66, Instruction{InstType::LD,   "ld h,(ix+*)",  3, 19, Operand::H, Operand::indIXN});
	//! ld ixh,a
	//! ld ixl,b
	//! ld ixl,c
	//! ld ixl,d
	//! ld ixl,e
	//! ld ixl,h
	//! ld ixl,l
	map_inst.emplace(0xdd6e, Instruction{InstType::LD,   "ld l,(ix+*)",  3, 19, Operand::L, Operand::indIXN});
	map_inst.emplace(0xdd6f, Instruction{InstType::LD,   "ld ixl,a",     2,  8, Operand::IXL, Operand::A});

	map_inst.emplace(0xdd70, Instruction{InstType::LD,   "ld (ix+*),b",  3, 19, Operand::indIXN, Operand::B});
	map_inst.emplace(0xdd71, Instruction{InstType::LD,   "ld (ix+*),c",  3, 19, Operand::indIXN, Operand::C});
	map_inst.emplace(0xdd72, Instruction{InstType::LD,   "ld (ix+*),d",  3, 19, Operand::indIXN, Operand::D});
	map_inst.emplace(0xdd73, Instruction{InstType::LD,   "ld (ix+*),e",  3, 19, Operand::indIXN, Operand::E});
	map_inst.emplace(0xdd74, Instruction{InstType::LD,   "ld (ix+*),h",  3, 19, Operand::indIXN, Operand::H});
	map_inst.emplace(0xdd75, Instruction{InstType::LD,   "ld (ix+*),l",  3, 19, Operand::indIXN, Operand::L});
	map_inst.emplace(0xdd77, Instruction{InstType::LD,   "ld (ix+*),a",  3, 19, Operand::indIXN, Operand::A});
	//! ld a,ixh
	//! ld a,ixl
	map_inst.emplace(0xdd7e, Instruction{InstType::LD,   "ld a,(ix+*)",  3, 19, Operand::A, Operand::indIXN});

	map_inst.emplace(0xdd84, Instruction{InstType::ADD,  "add a,ixh",    2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xdd85, Instruction{InstType::ADD,  "add a,ixl",    2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xdd86, Instruction{InstType::ADD,  "add a,(ix+*)", 3, 19, Operand::A, Operand::indIXN});
	map_inst.emplace(0xdd8c, Instruction{InstType::ADC,  "adc a,ixh",    2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xdd8d, Instruction{InstType::ADC,  "adc a,ixl",    2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xdd8e, Instruction{InstType::ADC,  "adc a,(ix+*)", 3, 19, Operand::A, Operand::indIXN});

	map_inst.emplace(0xdd94, Instruction{InstType::SUB,  "sub a,ixh",    2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xdd95, Instruction{InstType::SUB,  "sub a,ixl",    2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xdd96, Instruction{InstType::SUB,  "sub (ix+*)",   3, 19, Operand::A, Operand::indIXN});
	map_inst.emplace(0xdd9c, Instruction{InstType::SBC,  "sbc a,ixh",    2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xdd9d, Instruction{InstType::SBC,  "sbc a,ixl",    2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xdd9e, Instruction{InstType::SBC,  "sbc a,(ix+*)", 3, 19, Operand::A, Operand::indIXN});

	map_inst.emplace(0xdda4, Instruction{InstType::AND,  "and ixh",      2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xdda5, Instruction{InstType::AND,  "and ixl",      2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xdda6, Instruction{InstType::AND,  "and (ix+*)",   3, 19, Operand::A, Operand::indIXN});
	map_inst.emplace(0xddac, Instruction{InstType::XOR,  "xor ixh",      2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xddad, Instruction{InstType::XOR,  "xor ixl",      2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xddae, Instruction{InstType::XOR,  "xor (ix+*)",   3, 19, Operand::A, Operand::indIXN});

	map_inst.emplace(0xddb4, Instruction{InstType::OR,   "or ixh",       2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xddb5, Instruction{InstType::OR,   "or ixl",       2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xddb6, Instruction{InstType::OR,   "or (ix+*)",    3, 19, Operand::A, Operand::indIXN});
	map_inst.emplace(0xddbc, Instruction{InstType::CP,   "cp ixh",       2,  8, Operand::A, Operand::IXH});
	map_inst.emplace(0xddbd, Instruction{InstType::CP,   "cp ixl",       2,  8, Operand::A, Operand::IXL});
	map_inst.emplace(0xddbe, Instruction{InstType::CP,   "cp (ix+*)",    3, 19, Operand::A, Operand::indIXN});

	map_inst.emplace(0xdde1, Instruction{InstType::POP,  "pop ix",       2, 14, Operand::IX});
	//! ex (sp),ix
	map_inst.emplace(0xdde5, Instruction{InstType::PUSH, "push ix",      2, 15, Operand::UNUSED, Operand::IX});
	map_inst.emplace(0xdde9, Instruction{InstType::JP,   "jp (ix)",      2,  8, Conditional::ALWAYS, Operand::PC, Operand::IX});

	map_inst.emplace(0xddf9, Instruction{InstType::LD,   "ld sp,ix",     2, 10, Operand::SP, Operand::IX});



	//! TOO MANY TO LIST THAT ARE MISSING
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
	//! inc iyh
	//! dec iyh
	//! ld iyh,* 
	map_inst.emplace(0xfd29, Instruction{InstType::ADD,  "add iy,iy",    2, 15, Operand::IY, Operand::IY});
	map_inst.emplace(0xfd2a, Instruction{InstType::LD,   "ld iy,(**)",   4, 20, Operand::IY, Operand::indNN});
	map_inst.emplace(0xfd2b, Instruction{InstType::DEC,  "dec iy",       2, 10, Operand::IY, Operand::ONE});
	//! inc iyl
	//! dec iyl
	//! ld iyl,*

	map_inst.emplace(0xfd34, Instruction{InstType::INC,  "inc (iy+*)",   3, 23, Operand::indIYN, Operand::ONE});
	map_inst.emplace(0xfd35, Instruction{InstType::DEC,  "dec (iy+*)",   3, 23, Operand::indIYN, Operand::ONE});
	map_inst.emplace(0xfd36, Instruction{InstType::LD,   "ld (iy+*),*",  4, 19, Operand::indIYN, Operand::N});
	map_inst.emplace(0xfd39, Instruction{InstType::ADD,  "add iy,sp",    2, 15, Operand::IY, Operand::SP});

	//! ld b,iyh
	//! ld b,iyl
	map_inst.emplace(0xfd46, Instruction{InstType::LD,   "ld b,(iy+*)",  3, 19, Operand::B, Operand::indIYN});
	//! ld c,iyh
	//! ld c,iyl
	map_inst.emplace(0xfd4e, Instruction{InstType::LD,   "ld c,(iy+*)",  3, 19, Operand::C, Operand::indIYN});

	//! ld d,iyh
	//! ld d,iyl
	map_inst.emplace(0xfd56, Instruction{InstType::LD,   "ld d,(iy+*)",  3, 19, Operand::D, Operand::indIYN});
	//! ld e,iyh
	//! ld e,iyl
	map_inst.emplace(0xfd5e, Instruction{InstType::LD,   "ld e,(iy+*)",  3, 19, Operand::E, Operand::indIYN});

	//! ld iyh,b
	//! ld iyh,c
	//! ld iyh,d
	//! ld iyh,e
	//! ld iyh,h
	//! ld iyh,l
	map_inst.emplace(0xfd66, Instruction{InstType::LD,   "ld h,(iy+*)",  3, 19, Operand::H, Operand::indIYN});
	//! ld iyh,a
	//! ld iyl,b
	//! ld iyl,c
	//! ld iyl,d
	//! ld iyl,e
	//! ld iyl,h
	//! ld iyl,l
	map_inst.emplace(0xfd6e, Instruction{InstType::LD,   "ld j,(iy+*)",  3, 19, Operand::L, Operand::indIYN});
	//! ld iyl,a

	map_inst.emplace(0xfd70, Instruction{InstType::LD,   "ld (iy+*),b",  3, 19, Operand::indIYN, Operand::B});
	map_inst.emplace(0xfd71, Instruction{InstType::LD,   "ld (iy+*),c",  3, 19, Operand::indIYN, Operand::C});
	map_inst.emplace(0xfd72, Instruction{InstType::LD,   "ld (iy+*),d",  3, 19, Operand::indIYN, Operand::D});
	map_inst.emplace(0xfd73, Instruction{InstType::LD,   "ld (iy+*),e",  3, 19, Operand::indIYN, Operand::E});
	map_inst.emplace(0xfd74, Instruction{InstType::LD,   "ld (iy+*),h",  3, 19, Operand::indIYN, Operand::H});
	map_inst.emplace(0xfd75, Instruction{InstType::LD,   "ld (iy+*),l",  3, 19, Operand::indIYN, Operand::L});
	map_inst.emplace(0xfd77, Instruction{InstType::LD,   "ld (iy+*),a",  3, 19, Operand::indIYN, Operand::A});
	//! ld a,iyh
	//! ld a,iyl
	map_inst.emplace(0xfd7e, Instruction{InstType::LD,   "ld a,(iy+*)",  3, 19, Operand::A, Operand::indIYN});

	map_inst.emplace(0xfd84, Instruction{InstType::ADD,  "add a,iyh",    2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfd85, Instruction{InstType::ADD,  "add a,iyl",    2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfd86, Instruction{InstType::ADD,  "add a,(iy+*)", 3, 19, Operand::A, Operand::indIYN});
	map_inst.emplace(0xfd8c, Instruction{InstType::ADC,  "adc a,iyh",    2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfd8d, Instruction{InstType::ADC,  "adc a,iyl",    2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfd8e, Instruction{InstType::ADC,  "adc a,(iy+*)", 3, 19, Operand::A, Operand::indIYN});

	map_inst.emplace(0xfd94, Instruction{InstType::SUB,  "sub a,iyh",    2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfd95, Instruction{InstType::SUB,  "sub a,iyl",    2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfd96, Instruction{InstType::SUB,  "sub (iy+*)",   3, 19, Operand::A, Operand::indIYN});
	map_inst.emplace(0xfd9c, Instruction{InstType::SBC,  "sbc a,iyh",    2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfd9d, Instruction{InstType::SBC,  "sbc a,iyl",    2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfd9e, Instruction{InstType::SBC,  "sbc a,(iy+*)", 3, 19, Operand::A, Operand::indIYN});

	map_inst.emplace(0xfda4, Instruction{InstType::AND,  "and iyh",      2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfda5, Instruction{InstType::AND,  "and iyl",      2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfda6, Instruction{InstType::AND,  "and (iy+*)",   3, 19, Operand::A, Operand::indIYN});
	map_inst.emplace(0xfdac, Instruction{InstType::XOR,  "xor iyh",      2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfdad, Instruction{InstType::XOR,  "xor iyl",      2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfdae, Instruction{InstType::XOR,  "xor (iy+*)",   3, 19, Operand::A, Operand::indIYN});

	map_inst.emplace(0xfdb4, Instruction{InstType::OR,   "or iyh",       2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfdb5, Instruction{InstType::OR,   "or iyl",       2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfdb6, Instruction{InstType::OR,   "or (iy+*)",    3, 19, Operand::A, Operand::indIYN});
	map_inst.emplace(0xfdbc, Instruction{InstType::CP,   "cp iyh",       2, 8,  Operand::A, Operand::IYH});
	map_inst.emplace(0xfdbd, Instruction{InstType::CP,   "cp iyl",       2, 8,  Operand::A, Operand::IYL});
	map_inst.emplace(0xfdbe, Instruction{InstType::CP,   "cp (iy+*)",    3, 19, Operand::A, Operand::indIYN});

	map_inst.emplace(0xfde1, Instruction{InstType::POP,  "pop iy",       2, 14, Operand::IY});
	//! ex (sp),iy
	map_inst.emplace(0xfde5, Instruction{InstType::PUSH, "push iy",      2, 15, Operand::UNUSED, Operand::IY});
	map_inst.emplace(0xfde9, Instruction{InstType::JP,   "jp (iy)",      2,  8, Conditional::ALWAYS, Operand::PC, Operand::IY});

	map_inst.emplace(0xfdf9, Instruction{InstType::LD,   "ld sp,iy",     2, 10, Operand::SP, Operand::IY});


	
	//! TOO MANY TO LIST THAT ARE MISSING
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

void init_map_rom()
{
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
	map_rom.emplace(0x155d, "MAIN-ADD");
	map_rom.emplace(0x157d, "MAIN-ADD1");
	map_rom.emplace(0x15ab, "MAIN-ADD2");
	map_rom.emplace(0x15d4, "WAIT-KEY");
	map_rom.emplace(0x15de, "WAIT-KEY1");
	map_rom.emplace(0x15e4, "REPORT-8");
	map_rom.emplace(0x15e6, "INPUT-AD");
	map_rom.emplace(0x15ef, "OUT-CODE");
	map_rom.emplace(0x15f2, "PRINT-A-2");
	map_rom.emplace(0x15f7, "CALL-SUB");
	map_rom.emplace(0x1601, "CHAN-OPEN");
	map_rom.emplace(0x160e, "REPORT-O");
	map_rom.emplace(0x1610, "CHAN-OP-1");
	map_rom.emplace(0x1615, "CHAN-FLAG");
	map_rom.emplace(0x162c, "CALL-JUMP");
	map_rom.emplace(0x1634, "CHAN-K");
	map_rom.emplace(0x1642, "CHAN-S");
	map_rom.emplace(0x1646, "CHAN-S-1");
	map_rom.emplace(0x164d, "CHAN-P");
	map_rom.emplace(0x1652, "ONE-SPACE");
	map_rom.emplace(0x1655, "MAKE-ROOM");
	map_rom.emplace(0x1664, "POINTERS");
	map_rom.emplace(0x166b, "PTR-NEXT");
	map_rom.emplace(0x167f, "PTR-DONE");
	map_rom.emplace(0x168f, "LINE-ZERO");
	map_rom.emplace(0x1691, "LINE-NO-A");
	map_rom.emplace(0x1695, "LINE-NO");
	map_rom.emplace(0x169e, "RESERVE");
	map_rom.emplace(0x16b0, "SET-MIN");
	map_rom.emplace(0x16bf, "SET-WORK");
	map_rom.emplace(0x16c5, "SET-STK");
	map_rom.emplace(0x16d4, "REC-EDIT");
	map_rom.emplace(0x16dB, "INDEXER-1");
	map_rom.emplace(0x16dc, "INDEXER");
	map_rom.emplace(0x16e5, "CLOSE");
	map_rom.emplace(0x16fc, "CLOSE-1");
	map_rom.emplace(0x1701, "CLOSE-2");
	map_rom.emplace(0x171c, "CLOSE-STR");
	map_rom.emplace(0x171e, "STR-DATA");
	map_rom.emplace(0x1725, "REPORT-O");
	map_rom.emplace(0x1727, "STR-DATA1");
	map_rom.emplace(0x1736, "OPEN");
	map_rom.emplace(0x1756, "OPEN-1");
	map_rom.emplace(0x175d, "OPEN-2");
	map_rom.emplace(0x1765, "REPORT-F");
	map_rom.emplace(0x1767, "OPEN-3");
	map_rom.emplace(0x1781, "OPEN-K");
	map_rom.emplace(0x1785, "OPEN-S");
	map_rom.emplace(0x1789, "OPEN-P");
	map_rom.emplace(0x178B, "OPEN-END");
	map_rom.emplace(0x1793, "CAT-ETC.");
	map_rom.emplace(0x1795, "AUTO-LIST");
	map_rom.emplace(0x17ce, "AUTO-L-1");
	map_rom.emplace(0x17e1, "AUTO-L-2");
	map_rom.emplace(0x17e4, "AUTO-L-3");
	map_rom.emplace(0x17ed, "AUTO-L-4");
	map_rom.emplace(0x17f5, "LLIST");
	map_rom.emplace(0x17f9, "LIST");
	map_rom.emplace(0x17fb, "LIST-1");
	map_rom.emplace(0x1814, "LIST-2");
	map_rom.emplace(0x181a, "LIST-3");
	map_rom.emplace(0x181f, "LIST-4");
	map_rom.emplace(0x1822, "LIST-5");
	map_rom.emplace(0x1833, "LIST-ALL");
	map_rom.emplace(0x1835, "LIST-ALL-1");
	map_rom.emplace(0x1855, "OUT-LINE");
	map_rom.emplace(0x1865, "OUT-LINE1");
	map_rom.emplace(0x187d, "OUT-LINE2");
	map_rom.emplace(0x1881, "OUT-LINE3");
	map_rom.emplace(0x1894, "OUT-LINE4");
	map_rom.emplace(0x18a1, "OUT-LINE5");
	map_rom.emplace(0x18b4, "OUT-LINE6");
	map_rom.emplace(0x18b6, "NUMBER");
	map_rom.emplace(0x18c1, "OUT-FLASH");
	map_rom.emplace(0x18e1, "OUT-CURS");
	map_rom.emplace(0x18f3, "OUT-C-1");
	map_rom.emplace(0x1909, "OUT-C-2");
	map_rom.emplace(0x190f, "LN-FETCH");
	map_rom.emplace(0x191c, "LN-STORE");
	map_rom.emplace(0x1925, "OUT-SP-2");
	map_rom.emplace(0x192a, "OUT-SP-NO");
	map_rom.emplace(0x192b, "OUT-SP-1");
	map_rom.emplace(0x1937, "OUT-CHAR");
	map_rom.emplace(0x195a, "OUT-CH-1");
	map_rom.emplace(0x1968, "OUT-CH-2");
	map_rom.emplace(0x196c, "OUT-CH-3");
	map_rom.emplace(0x196e, "LINE-ADDR");
	map_rom.emplace(0x1974, "LINE-AD-1");
	map_rom.emplace(0x1980, "CP-LINES");
	map_rom.emplace(0x198b, "EACH-STMT");
	map_rom.emplace(0x1990, "EACH-S-1");
	map_rom.emplace(0x1998, "EACH-S-2");
	map_rom.emplace(0x199a, "EACH-S-3");
	map_rom.emplace(0x19a5, "EACH-S-4");
	map_rom.emplace(0x19ad, "EACH-S-5");
	map_rom.emplace(0x19b1, "EACH-S-6");
	map_rom.emplace(0x19b8, "NEXT-ONE");
	map_rom.emplace(0x19c7, "NEXT-O-1");
	map_rom.emplace(0x19ce, "NEXT-O-2");
	map_rom.emplace(0x19d5, "NEXT-O-3");
	map_rom.emplace(0x19d6, "NEXT-O-4");
	map_rom.emplace(0x19db, "NEXT-O-5");
	map_rom.emplace(0x19dd, "DIFFER");
	map_rom.emplace(0x19e5, "RECLAIM-1");
	map_rom.emplace(0x19e8, "RECLAIM-2");
	map_rom.emplace(0x19fb, "E-LINE-NO");
	map_rom.emplace(0x1a15, "E-L-1");
	map_rom.emplace(0x1a1b, "OUT-NUM-1");
	map_rom.emplace(0x1a28, "OUT-NUM-2");
	map_rom.emplace(0x1a30, "OUT-NUM-3");
	map_rom.emplace(0x1a42, "OUT-NUM-4");
	map_rom.emplace(0x1a7a, "P-LET");
	map_rom.emplace(0x1a7d, "P-GO-TO");
	map_rom.emplace(0x1a81, "P-IF");
	map_rom.emplace(0x1a86, "P-GO-SUB");
	map_rom.emplace(0x1a8a, "P-STOP");
	map_rom.emplace(0x1a8d, "P-RETURN");
	map_rom.emplace(0x1a90, "P-FOR");
	map_rom.emplace(0x1a98, "P-NEXT");
	map_rom.emplace(0x1a9c, "P-PRINT");
	map_rom.emplace(0x1a9f, "P-INPUT");
	map_rom.emplace(0x1aa2, "P-DIM");
	map_rom.emplace(0x1aa5, "P-REM");
	map_rom.emplace(0x1aa8, "P-NEW");
	map_rom.emplace(0x1aab, "P-RUN");
	map_rom.emplace(0x1aae, "P-LIST");
	map_rom.emplace(0x1ab1, "P-POKE");
	map_rom.emplace(0x1ab5, "P-RANDOM");
	map_rom.emplace(0x1ab8, "P-CONT");
	map_rom.emplace(0x1abb, "P-CLEAR");
	map_rom.emplace(0x1abe, "P-CLS");
	map_rom.emplace(0x1ac1, "P-PLOT");
	map_rom.emplace(0x1ac5, "P-PAUSE");
	map_rom.emplace(0x1ac9, "P-READ");
	map_rom.emplace(0x1acc, "P-DATA");
	map_rom.emplace(0x1acf, "P-RESTORE");
	map_rom.emplace(0x1ad2, "P-DRAW");
	map_rom.emplace(0x1ad6, "P-COPY");
	map_rom.emplace(0x1ad9, "P-LPRINT");
	map_rom.emplace(0x1adc, "P-LLIST");
	map_rom.emplace(0x1adf, "P-SAVE");
	map_rom.emplace(0x1ae0, "P-LOAD");
	map_rom.emplace(0x1ae1, "P-VERIFY");
	map_rom.emplace(0x1ae2, "P-MERGE");
	map_rom.emplace(0x1ae3, "P-BEEP");
	map_rom.emplace(0x1ae7, "P-CIRCLE");
	map_rom.emplace(0x1aeb, "P-INK");
	map_rom.emplace(0x1aec, "P-PAPER");
	map_rom.emplace(0x1aed, "P-FLASH");
	map_rom.emplace(0x1aee, "P-BRIGHT");
	map_rom.emplace(0x1aef, "P-INVERSE");
	map_rom.emplace(0x1af0, "P-OVER");
	map_rom.emplace(0x1af1, "P-OUT");
	map_rom.emplace(0x1af5, "P-BORDER");
	map_rom.emplace(0x1af9, "P-DEF-FN");
	map_rom.emplace(0x1afc, "P-OPEN");
	map_rom.emplace(0x1b02, "P-CLOSE");
	map_rom.emplace(0x1b06, "P-FORMAT");
	map_rom.emplace(0x1b0a, "P-MOVE");
	map_rom.emplace(0x1b10, "P-ERASE");
	map_rom.emplace(0x1b14, "P-CAT");
	map_rom.emplace(0x1b17, "LINE-SCAN");
	map_rom.emplace(0x1b28, "STMT-LOOP");
	map_rom.emplace(0x1b29, "STMT-L-1");
	map_rom.emplace(0x1b52, "SCAN-LOOP");
	map_rom.emplace(0x1b55, "GET-PARAM");
	map_rom.emplace(0x1b6f, "SEPARATOR");
	map_rom.emplace(0x1b76, "STMT-RET");
	map_rom.emplace(0x1b7b, "REPORT-L");
	map_rom.emplace(0x1b7d, "STMT-R-1");
	map_rom.emplace(0x1b8a, "LINE-RUN");
	map_rom.emplace(0x1b9e, "LINE-NEW");
	map_rom.emplace(0x1bb0, "REPORT-0");
	map_rom.emplace(0x1bb2, "REM");
	map_rom.emplace(0x1bb3, "LINE-END");
	map_rom.emplace(0x1bbf, "LINE-USE");
	map_rom.emplace(0x1bd1, "NEXT-LINE");
	map_rom.emplace(0x1bec, "REPORT-N");
	map_rom.emplace(0x1bee, "CHECK-END");
	map_rom.emplace(0x1bf4, "STMT-NEXT");
	map_rom.emplace(0x1c0d, "CLASS-03");
	map_rom.emplace(0x1c10, "CLASS-00");
	map_rom.emplace(0x1c11, "CLASS-05");
	map_rom.emplace(0x1c16, "JUMP-C-R");
	map_rom.emplace(0x1c1f, "CLASS-01");
	map_rom.emplace(0x1c22, "VAR-A-1");
	map_rom.emplace(0x1c2e, "REPORT-2");
	map_rom.emplace(0x1c30, "VAR-A-2");
	map_rom.emplace(0x1c46, "VAR-A-3");
	map_rom.emplace(0x1c4e, "CLASS-02");
	map_rom.emplace(0x1c56, "VAL-FET-1");
	map_rom.emplace(0x1c59, "VAL-FET-2");
	map_rom.emplace(0x1c6c, "CLASS-04");
	map_rom.emplace(0x1c79, "NEXT-2NUM");
	map_rom.emplace(0x1c7a, "EXPT-2NUM");
	map_rom.emplace(0x1c82, "EXPT-1NUM");
	map_rom.emplace(0x1c8a, "REPORT-C");
	map_rom.emplace(0x1c8c, "EXPT-EXP");
	map_rom.emplace(0x1c96, "PERMS");
	map_rom.emplace(0x1cbe, "CLASS-09");
	map_rom.emplace(0x1cd6, "CL-09-1");
	map_rom.emplace(0x1cdb, "CLASS-0B");
	map_rom.emplace(0x1cde, "FETCH-NUM");
	map_rom.emplace(0x1ce6, "USE-ZERO");
	map_rom.emplace(0x1cee, "STOP");
	map_rom.emplace(0x1cf0, "IF");
	map_rom.emplace(0x1d00, "IF-1");
	map_rom.emplace(0x1d03, "FOR");
	map_rom.emplace(0x1d10, "F-USE-1");
	map_rom.emplace(0x1d16, "F-REORDER");
	map_rom.emplace(0x1d34, "F-L&S");
	map_rom.emplace(0x1d64, "F-LOOP");
	map_rom.emplace(0x1d7C, "F-FOUND");
	map_rom.emplace(0x1d84, "REPORT-I");
	map_rom.emplace(0x1d8b, "LOOK-P-1");
	map_rom.emplace(0x1da3, "LOOK-P-2");
	map_rom.emplace(0x1dab, "NEXT");
	map_rom.emplace(0x1dd8, "REPORT-1");
	map_rom.emplace(0x1dda, "NEXT-LOOP");
	map_rom.emplace(0x1de2, "NEXT-1");
	map_rom.emplace(0x1de9, "NEXT-2");
	map_rom.emplace(0x1dec, "READ-3");
	map_rom.emplace(0x1ded, "READ");
	map_rom.emplace(0x1e08, "REPORT-E");
	map_rom.emplace(0x1e0a, "READ-1");
	map_rom.emplace(0x1e1e, "READ-2");
	map_rom.emplace(0x1e2c, "DATA-1");
	map_rom.emplace(0x1e37, "DATA-2");
	map_rom.emplace(0x1e39, "PASS-BY");
	map_rom.emplace(0x1e42, "RESTORE");
	map_rom.emplace(0x1e45, "REST-RUN");
	map_rom.emplace(0x1e4f, "RANDOMIZE");
	map_rom.emplace(0x1e5a, "RAND-1");
	map_rom.emplace(0x1e5f, "CONTINUE");
	map_rom.emplace(0x1e67, "GO-TO");
	map_rom.emplace(0x1e73, "GO-TO-2");
	map_rom.emplace(0x1e7a, "OUT");
	map_rom.emplace(0x1e80, "POKE");
	map_rom.emplace(0x1e85, "TWO-PARAM");
	map_rom.emplace(0x1e8e, "TWO-P-1");
	map_rom.emplace(0x1e94, "FIND-INT1");
	map_rom.emplace(0x1e99, "FIND-INT2");
	map_rom.emplace(0x1e9c, "FIND-I-1");
	map_rom.emplace(0x1e9f, "REPORT-B");
	map_rom.emplace(0x1ea1, "RUN");
	map_rom.emplace(0x1eaf, "CLEAR-RUN");
	map_rom.emplace(0x1eb7, "CLEAR-1");
	map_rom.emplace(0x1eda, "REPORT-M");
	map_rom.emplace(0x1edc, "CLEAR-2");
	map_rom.emplace(0x1eed, "GO-SUB");
	map_rom.emplace(0x1f05, "TEST-ROOM");
	map_rom.emplace(0x1f15, "REPORT-4");
	map_rom.emplace(0x1f1a, "FREE-MEM");
	map_rom.emplace(0x1f23, "RETURN");
	map_rom.emplace(0x1f36, "REPORT-7");
	map_rom.emplace(0x1f3a, "PAUSE");
	map_rom.emplace(0x1f3d, "PAUSE-1");
	map_rom.emplace(0x1f49, "PAUSE-2");
	map_rom.emplace(0x1f4f, "PAUSE-END");
	map_rom.emplace(0x1f54, "BREAK-KEY");
	map_rom.emplace(0x1f6a, "DEF-FN-1");
	map_rom.emplace(0x1f7d, "DEF-FN-2");
	map_rom.emplace(0x1f86, "DEF-FN-3");
	map_rom.emplace(0x1f89, "DEF-FN-4");
	map_rom.emplace(0x1f94, "DEF-FN-5");
	map_rom.emplace(0x1fa6, "DEF-FN-6");
	map_rom.emplace(0x1fbd, "DEF-FN-7");
	map_rom.emplace(0x1fc3, "UNSTACK-Z");
	map_rom.emplace(0x1fc9, "LPRINT");
	map_rom.emplace(0x1fcd, "PRINT");
	map_rom.emplace(0x1fcf, "PRINT-1");
	map_rom.emplace(0x1fdf, "PRINT-2");
	map_rom.emplace(0x1fe5, "PRINT-3");
	map_rom.emplace(0x1ff2, "PRINT-4");
	map_rom.emplace(0x1ff5, "PRINT-CR");
	map_rom.emplace(0x1ffc, "PR-ITEM-1");
	map_rom.emplace(0x200e, "PR-ITEM-2");
	map_rom.emplace(0x201e, "PR-AT-TAB");
	map_rom.emplace(0x2024, "PR-ITEM-3");
	map_rom.emplace(0x203c, "PR-STRING");
	map_rom.emplace(0x2045, "PR-END-Z");
	map_rom.emplace(0x2048, "PR-ST-END");
	map_rom.emplace(0x204e, "PR-POSN-1");
	map_rom.emplace(0x2061, "PR-POSN-2");
	map_rom.emplace(0x2067, "PR-POSN-3");
	map_rom.emplace(0x206e, "PR-POSN-4");
	map_rom.emplace(0x2070, "STR-ALTER");
	map_rom.emplace(0x2089, "INPUT");
	map_rom.emplace(0x2096, "INPUT-1");
	map_rom.emplace(0x20ad, "INPUT-2");
	map_rom.emplace(0x20c1, "IN-ITEM-1");
	map_rom.emplace(0x20d8, "IN-ITEM-2");
	map_rom.emplace(0x20ed, "IN-ITEM-3");
	map_rom.emplace(0x20fa, "IN-PROMPT");
	map_rom.emplace(0x211a, "IN-PR-1");
	map_rom.emplace(0x211c, "IN-PR-2");
	map_rom.emplace(0x2129, "IN-PR-3");
	map_rom.emplace(0x213a, "IN-VAR-1");
	map_rom.emplace(0x2148, "IN-VAR-2");
	map_rom.emplace(0x215e, "IN-VAR-3");
	map_rom.emplace(0x2161, "IN-VAR-4");
	map_rom.emplace(0x2174, "IN-VAR-5");
	map_rom.emplace(0x219b, "IN-VAR-6");
	map_rom.emplace(0x21af, "IN-NEXT-1");
	map_rom.emplace(0x21b2, "IN-NEXT-2");
	map_rom.emplace(0x21b9, "IN-ASSIGN");
	map_rom.emplace(0x21ce, "REPORT-C");
	map_rom.emplace(0x21d0, "IN-STOP");
	map_rom.emplace(0x21d4, "REPORT-H");
	map_rom.emplace(0x21d6, "IN-CHAN-K");
	map_rom.emplace(0x21e1, "CO-TEMP-1");
	map_rom.emplace(0x21e2, "CO-TEMP-2");
	map_rom.emplace(0x21f2, "CO-TEMP-3");
	map_rom.emplace(0x21fc, "CO-TEMP-4");
	map_rom.emplace(0x2211, "CO-TEMP-5");
	map_rom.emplace(0x2228, "CO-TEMP-6");
	map_rom.emplace(0x2234, "CO-TEMP-7");
	map_rom.emplace(0x223e, "CO-TEMP-8");
	map_rom.emplace(0x2244, "REPORT-K");
	map_rom.emplace(0x2246, "CO-TEMP-9");
	map_rom.emplace(0x2257, "CO-TEMP-A");
	map_rom.emplace(0x2258, "CO-TEMP-B");
	map_rom.emplace(0x226c, "CO-CHANGE");
	map_rom.emplace(0x2273, "CO-TEMP-C");
	map_rom.emplace(0x227d, "CO-TEMP-D");
	map_rom.emplace(0x2287, "CO-TEMP-E");
	map_rom.emplace(0x2294, "BORDER");
	map_rom.emplace(0x22a6, "BORDER-1");
	map_rom.emplace(0x22aa, "PIXEL-ADD");
	map_rom.emplace(0x22cb, "POINT-SUB");
	map_rom.emplace(0x22d4, "POINT-LP");
	map_rom.emplace(0x22dc, "PLOT");
	map_rom.emplace(0x22e5, "PLOT-SUB");
	map_rom.emplace(0x22f0, "PLOT-LOOP");
	map_rom.emplace(0x22fd, "PL-TST-IN");
	map_rom.emplace(0x2303, "PLOT-END");
	map_rom.emplace(0x2307, "STK-TO-BC");
	map_rom.emplace(0x2314, "STK-TO-A");
	map_rom.emplace(0x2320, "CIRCLE");
	map_rom.emplace(0x233b, "C-R-GRE-1");
	map_rom.emplace(0x235a, "C-ARC-GE1");
	map_rom.emplace(0x2382, "DRAW");
	map_rom.emplace(0x238d, "DR-3-PRMS");
	map_rom.emplace(0x23a3, "DR-SIN-NZ");
	map_rom.emplace(0x23c1, "DR-PRMS");
	map_rom.emplace(0x2420, "DRW-STEPS");
	map_rom.emplace(0x2425, "ARC-LOOP");
	map_rom.emplace(0x2439, "ARC-START");
	map_rom.emplace(0x245f, "ARC-END");
	map_rom.emplace(0x2477, "LINE-DRAW");
	map_rom.emplace(0x247d, "CD-PRMS1");
	map_rom.emplace(0x2495, "USE-252");
	map_rom.emplace(0x2497, "DRAW-SAVE");
	map_rom.emplace(0x24b7, "DRAW-LINE");
	map_rom.emplace(0x24c4, "DL-X-GE-Y");
	map_rom.emplace(0x24cb, "DL-LARGER");
	map_rom.emplace(0x24ce, "D-L-LOOP");
	map_rom.emplace(0x24d4, "D-L-DIAG");
	map_rom.emplace(0x24db, "D-L-HR-VT");
	map_rom.emplace(0x24df, "D-L-STEP");
	map_rom.emplace(0x24ec, "D-L-PLOT");
	map_rom.emplace(0x24f7, "D-L-RANGE");
	map_rom.emplace(0x24f9, "REPORT-B");
	map_rom.emplace(0x24fb, "SCANNING");
	map_rom.emplace(0x24ff, "S-LOOP-1");
	map_rom.emplace(0x250f, "S-QUOTE-S");
	map_rom.emplace(0x2522, "S-2-COORD");
	map_rom.emplace(0x252d, "S-RPORT-C");
	map_rom.emplace(0x2530, "SYNTAX-Z");
	map_rom.emplace(0x2535, "S-SCRN$-S");
	map_rom.emplace(0x254f, "S-SCRN-LP");
	map_rom.emplace(0x255a, "S-SC-MTCH");
	map_rom.emplace(0x255d, "S-SC-ROWS");
	map_rom.emplace(0x2573, "S-SCR-NXT");
	map_rom.emplace(0x257d, "S-SCR-STO");
	map_rom.emplace(0x2580, "S-ATTR-S");
	map_rom.emplace(0x25af, "S-U-PLUS");
	map_rom.emplace(0x25b3, "S-QUOTE");
	map_rom.emplace(0x25be, "S-Q-AGAIN");
	map_rom.emplace(0x25cb, "S-Q-COPY");
	map_rom.emplace(0x25d9, "S-Q-PRMS");
	map_rom.emplace(0x25db, "S-STRING");
	map_rom.emplace(0x25e8, "S-BRACKET");
	map_rom.emplace(0x25f5, "S-FN");
	map_rom.emplace(0x25f8, "S-RND");
	map_rom.emplace(0x2625, "S-RND-END");
	map_rom.emplace(0x2627, "S-PI");
	map_rom.emplace(0x2630, "S-PI-END");
	map_rom.emplace(0x2634, "S-INKEY$");
	map_rom.emplace(0x2660, "S-IK$-STK");
	map_rom.emplace(0x2665, "S-INK$-EN");
	map_rom.emplace(0x2668, "S-SCREEN$");
	map_rom.emplace(0x2672, "S-ATTR");
	map_rom.emplace(0x267b, "S-POINT");
	map_rom.emplace(0x26b4, "S-ALPHNUM");
	map_rom.emplace(0x268d, "S-DECIMAL");
	map_rom.emplace(0x26b5, "S-STK-DEC");
	map_rom.emplace(0x26b6, "S-SD-SKIP");
	map_rom.emplace(0x26c3, "S-NUMERIC");
	map_rom.emplace(0x26c9, "S-LETTER");
	map_rom.emplace(0x26dd, "S-CONT1");
	map_rom.emplace(0x26df, "S-NEGATE");
	map_rom.emplace(0x2707, "S-NO-TO-S");
	map_rom.emplace(0x270d, "S-PUSH-PO");
	map_rom.emplace(0x2712, "S-CONT-2");
	map_rom.emplace(0x2713, "S-CONT-3");
	map_rom.emplace(0x2723, "S-OPERTR");
	map_rom.emplace(0x2734, "S-LOOP");
	map_rom.emplace(0x274c, "S-STK-LST");
	map_rom.emplace(0x275b, "S-SYNTEST");
	map_rom.emplace(0x2761, "S-RPORT-C");
	map_rom.emplace(0x2764, "S-RUNTEST");
	map_rom.emplace(0x2770, "S-LOOPEND");
	map_rom.emplace(0x2773, "S-TIGHTER");
	map_rom.emplace(0x2788, "S-NOT-AND");
	map_rom.emplace(0x2790, "S-NEXT");
	map_rom.emplace(0x27bd, "S-FN-SBRN");
	map_rom.emplace(0x27d0, "SF-BRKT-1");
	map_rom.emplace(0x27d9, "SF-ARGMTS");
	map_rom.emplace(0x27e4, "SF-BRKT-2");
	map_rom.emplace(0x27e6, "SF-RPRT-C");
	map_rom.emplace(0x27e9, "SF-FLAG-6");
	map_rom.emplace(0x27f4, "SF-SYN-EN");
	map_rom.emplace(0x27f7, "SF-RUN");
	map_rom.emplace(0x2802, "SF-ARGMT1");
	map_rom.emplace(0x2808, "SF-FND-DF");
	map_rom.emplace(0x2812, "REPORT-P");
	map_rom.emplace(0x2814, "SF-CP-DEF");
	map_rom.emplace(0x2825, "SF-NOT-FD");
	map_rom.emplace(0x2831, "SF-VALUES");
	map_rom.emplace(0x2843, "SF-ARG-LP");
	map_rom.emplace(0x2852, "SF-ARG-VL");
	map_rom.emplace(0x2885, "SF-R-BR-2");
	map_rom.emplace(0x288b, "REPORT-Q");
	map_rom.emplace(0x288d, "SF-VALUE");
	map_rom.emplace(0x28ab, "FN-SKPOVR");
	map_rom.emplace(0x28b2, "LOOK-VARS");
	map_rom.emplace(0x28d4, "V-CHAR");
	map_rom.emplace(0x28de, "V-STR-VAR");
	map_rom.emplace(0x28e3, "V-TEST-FN");
	map_rom.emplace(0x28ef, "V-RUN/SYN");
	map_rom.emplace(0x28fd, "V-RUN");
	map_rom.emplace(0x2900, "V-EACH");
	map_rom.emplace(0x2912, "V-MATCHES");
	map_rom.emplace(0x2913, "V-SPACES");
	map_rom.emplace(0x2929, "V-GET-PTR");
	map_rom.emplace(0x292a, "V-NEXT");
	map_rom.emplace(0x2932, "V-80-BYTE");
	map_rom.emplace(0x2934, "V-SYNTAX");
	map_rom.emplace(0x293e, "V-FOUND-1");
	map_rom.emplace(0x293f, "V-FOUND-2");
	map_rom.emplace(0x2943, "V-PASS");
	map_rom.emplace(0x294b, "V-END");
	map_rom.emplace(0x2951, "STK-F-ARG");
	map_rom.emplace(0x295a, "SFA-LOOP");
	map_rom.emplace(0x296b, "SFA-CP-VR");
	map_rom.emplace(0x2981, "SFA-MATCH");
	map_rom.emplace(0x2991, "SFA-END");
	map_rom.emplace(0x2996, "STK-VAR");
	map_rom.emplace(0x29a1, "SV-SIMPLE$");
	map_rom.emplace(0x29ae, "SV-ARRAYS");
	map_rom.emplace(0x29c0, "SV-PTR");
	map_rom.emplace(0x29c3, "SV-COMMA");
	map_rom.emplace(0x29d8, "SV-CLOSE");
	map_rom.emplace(0x29e0, "SV-CH-ADD");
	map_rom.emplace(0x29e7, "SV-COUNT");
	map_rom.emplace(0x29ea, "SV-LOOP");
	map_rom.emplace(0x29fb, "SV-MULT");
	map_rom.emplace(0x2a12, "SV-RPT-C");
	map_rom.emplace(0x2a20, "REPORT-3");
	map_rom.emplace(0x2a22, "SV-NUMBER");
	map_rom.emplace(0x2a2c, "SV-ELEM$");
	map_rom.emplace(0x2a45, "SV-SLICE");
	map_rom.emplace(0x2a48, "SV-DIM");
	map_rom.emplace(0x2a49, "SV-SLICE?");
	map_rom.emplace(0x2a52, "SLICING");
	map_rom.emplace(0x2a7a, "SL-RPT-C");
	map_rom.emplace(0x2a81, "SL-SECOND");
	map_rom.emplace(0x2a94, "SL-DEFINE");
	map_rom.emplace(0x2aa8, "SL-OVER");
	map_rom.emplace(0x2aad, "SL-STORE");
	map_rom.emplace(0x2ab1, "STK-ST-0");
	map_rom.emplace(0x2ab2, "STK-STO-$");
	map_rom.emplace(0x2ab6, "STK-STORE");
	map_rom.emplace(0x2acc, "INT-EXP1");
	map_rom.emplace(0x2acd, "INT-EXP2");
	map_rom.emplace(0x2ae8, "I-CARRY");
	map_rom.emplace(0x2aeb, "I-RESTORE");
	map_rom.emplace(0x2aee, "DE,(DE+1)");
	map_rom.emplace(0x2af4, "GET-HL*DE");
	map_rom.emplace(0x2aff, "LET");
	map_rom.emplace(0x2b0b, "L-EACH-CH");
	map_rom.emplace(0x2b0c, "L-NO-SP");
	map_rom.emplace(0x2b1f, "L-TEST-CH");
	map_rom.emplace(0x2b29, "L-SPACES");
	map_rom.emplace(0x2b3e, "L-CHAR");
	map_rom.emplace(0x2b4f, "L-SINGLE");
	map_rom.emplace(0x2b59, "L-NUMERIC");
	map_rom.emplace(0x2b66, "L-EXISTS");
	map_rom.emplace(0x2b72, "L-DELETE$");
	map_rom.emplace(0x2b9b, "L-LENGTH");
	map_rom.emplace(0x2ba3, "L-IN-W/S");
	map_rom.emplace(0x2ba6, "L-ENTER");
	map_rom.emplace(0x2baf, "L-ADD$");
	map_rom.emplace(0x2bc0, "L-NEW$");
	map_rom.emplace(0x2bc6, "L-STRING");
	map_rom.emplace(0x2bea, "L-FIRST");
	map_rom.emplace(0x2bf1, "STK-FETCH");
	map_rom.emplace(0x2c02, "DIM");
	map_rom.emplace(0x2c05, "D-RPORT-C");
	map_rom.emplace(0x2c15, "D-RUN");
	map_rom.emplace(0x2c1f, "D-LETTER");
	map_rom.emplace(0x2c2d, "D-SIZE");
	map_rom.emplace(0x2c2e, "D-NO-LOOP");
	map_rom.emplace(0x2c7c, "DIM-CLEAR");
	map_rom.emplace(0x2c7f, "DIM-SIZES");
	map_rom.emplace(0x2c88, "ALPHANUM");
	map_rom.emplace(0x2c8d, "ALPHA");
	map_rom.emplace(0x2c9b, "DEC-TO-FP");
	map_rom.emplace(0x2ca2, "BIN-DIGIT");
	map_rom.emplace(0x2cb3, "BIN-END");
	map_rom.emplace(0x2cb8, "NOT-BIN");
	map_rom.emplace(0x2ccb, "DECIMAL");
	map_rom.emplace(0x2ccf, "DEC-RPT-C");
	map_rom.emplace(0x2cd5, "DEC-STO-1");
	map_rom.emplace(0x2cda, "NXT-DGT-1");
	map_rom.emplace(0x2ceb, "E-FORMAT");
	map_rom.emplace(0x2cf2, "SIGN-FLAG");
	map_rom.emplace(0x2cfe, "SIGN-DONE");
	map_rom.emplace(0x2cff, "ST-E-PART");
	map_rom.emplace(0x2d18, "E-FP-JUMP");
	map_rom.emplace(0x2d1b, "NUMERIC");
	map_rom.emplace(0x2d22, "STK-DIGIT");
	map_rom.emplace(0x2d28, "STACK-A");
	map_rom.emplace(0x2d2b, "STACK-BC");
	map_rom.emplace(0x2d3b, "INT-TO-FP");
	map_rom.emplace(0x2d40, "NXT-DGT-2");
	map_rom.emplace(0x2d4f, "E-TO-FP");
	map_rom.emplace(0x2d55, "E-SAVE");
	map_rom.emplace(0x2d60, "E-LOOP");
	map_rom.emplace(0x2d6d, "E-DIVSN");
	map_rom.emplace(0x2d6e, "E-FETCH");
	map_rom.emplace(0x2d71, "E-TST-END");
	map_rom.emplace(0x2d7b, "E-END");
	map_rom.emplace(0x2d7f, "INT-FETCH");
	map_rom.emplace(0x2d8c, "P-INT-STO");
	map_rom.emplace(0x2d8e, "INT-STORE");
	map_rom.emplace(0x2da2, "FP-TO-BC");
	map_rom.emplace(0x2dad, "FP-DELETE");
	map_rom.emplace(0x2dc1, "LOG(2^A)");
	map_rom.emplace(0x2dd5, "FP-TO-A");
	map_rom.emplace(0x2de1, "FP-A-END");
	map_rom.emplace(0x2de3, "PRINT-FP");
	map_rom.emplace(0x2df2, "PF-NEGTVE");
	map_rom.emplace(0x2df8, "PF-POSTVE");
	map_rom.emplace(0x2e01, "PF-LOOP");
	map_rom.emplace(0x2e1e, "PF-SAVE");
	map_rom.emplace(0x2e56, "PF-LARGE");
	map_rom.emplace(0x2e6f, "PF-MEDIUM");
	map_rom.emplace(0x2e7b, "PF-BITS");
	map_rom.emplace(0x2e8a, "PF-BYTES");
	map_rom.emplace(0x2ea1, "PF-DIGITS");
	map_rom.emplace(0x2ea9, "PF-INSERT");
	map_rom.emplace(0x2eb3, "PF-TEST-2");
	map_rom.emplace(0x2eb8, "PF-ALL-9");
	map_rom.emplace(0x2ecb, "PF-MORE");
	map_rom.emplace(0x2ecf, "PF-FRACTN");
	map_rom.emplace(0x2edf, "PF-FRN-LP");
	map_rom.emplace(0x2eec, "PF-FR-DGT");
	map_rom.emplace(0x2eef, "PF-FR-EXX");
	map_rom.emplace(0x2f0c, "PF-ROUND");
	map_rom.emplace(0x2f18, "PF-END-LP");
	map_rom.emplace(0x2f25, "PF-R-BACK");
	map_rom.emplace(0x2f2d, "PF-COUNT");
	map_rom.emplace(0x2f46, "PF-NOT-E");
	map_rom.emplace(0x2f4a, "PF-E-SBRN");
	map_rom.emplace(0x2f52, "PF-OUT-LP");
	map_rom.emplace(0x2f59, "PF-OUT-DT");
	map_rom.emplace(0x2f5e, "PF-DC-OUT");
	map_rom.emplace(0x2f64, "PF-DEC-0S");
	map_rom.emplace(0x2f6c, "PF-E-FRMT");
	map_rom.emplace(0x2f83, "PF-E-POS");
	map_rom.emplace(0x2f85, "PF-E-SIGN");
	map_rom.emplace(0x2f8b, "CA=10*A+C");
	map_rom.emplace(0x2f9b, "PREP-ADD");
	map_rom.emplace(0x2faf, "NEG-BYTE");
	map_rom.emplace(0x2fba, "FETCH-TWO");
	map_rom.emplace(0x2fdd, "SHIFT-FP");
	map_rom.emplace(0x2fe5, "ONE-SHIFT");
	map_rom.emplace(0x2ff9, "ADDEND-0");
	map_rom.emplace(0x2ffb, "ZEROS-4/5");
	map_rom.emplace(0x3004, "ADD-BACK");
	map_rom.emplace(0x300d, "ALL-ADDED");
	map_rom.emplace(0x300f, "SUBTRACT");
	map_rom.emplace(0x3014, "ADDITION");
	map_rom.emplace(0x303c, "ADDN-OFLW");
	map_rom.emplace(0x303e, "FULL-ADDN");
	map_rom.emplace(0x3055, "SHIFT-LEN");
	map_rom.emplace(0x307c, "TEST-NEG");
	map_rom.emplace(0x309f, "ADD-REP-6");
	map_rom.emplace(0x30a3, "END-COMPL");
	map_rom.emplace(0x30a5, "GO-NC-MLT");
	map_rom.emplace(0x30a9, "HL=HL*DE");
	map_rom.emplace(0x30b1, "HL-LOOP");
	map_rom.emplace(0x30bc, "HL-AGAIN");
	map_rom.emplace(0x30be, "HL-END");
	map_rom.emplace(0x30c0, "PREP-M/D");
	map_rom.emplace(0x30ca, "MULTIPLY");
	map_rom.emplace(0x30ea, "MULT-RSLT");
	map_rom.emplace(0x30ef, "MULT-OFLW");
	map_rom.emplace(0x30f0, "MULT-LONG");
	map_rom.emplace(0x3114, "MLT-LOOP");
	map_rom.emplace(0x311b, "NO-ADD");
	map_rom.emplace(0x3125, "STRT-MLT");
	map_rom.emplace(0x313b, "MAKE-EXPT");
	map_rom.emplace(0x313d, "DIVN-EXPT");
	map_rom.emplace(0x3146, "OFLW1-CLR");
	map_rom.emplace(0x3151, "OFLW2-CLR");
	map_rom.emplace(0x3155, "TEST-NORM");
	map_rom.emplace(0x3159, "NEAR-ZERO");
	map_rom.emplace(0x315d, "ZERO-RSLT");
	map_rom.emplace(0x315e, "SKIP-ZERO");
	map_rom.emplace(0x316c, "NORMALISE");
	map_rom.emplace(0x316e, "SHIFT-ONE");
	map_rom.emplace(0x3186, "NORML-NOW");
	map_rom.emplace(0x3195, "OFLOW-CLR");
	map_rom.emplace(0x31ad, "REPORT-6");
	map_rom.emplace(0x31af, "DIVISION");
	map_rom.emplace(0x31d2, "DIV-LOOP");
	map_rom.emplace(0x31d8, "DIV-34TH");
	map_rom.emplace(0x31e2, "DIV-START");
	map_rom.emplace(0x31f2, "SUBN-ONLY");
	map_rom.emplace(0x31f9, "NO-RSTORE");
	map_rom.emplace(0x31fa, "COUNT-ONE");
	map_rom.emplace(0x3214, "TRUNCATE");
	map_rom.emplace(0x3221, "T-GR-ZERO");
	map_rom.emplace(0x3233, "T-FIRST");
	map_rom.emplace(0x323f, "T-SMALL");
	map_rom.emplace(0x3252, "T-NUMERIC");
	map_rom.emplace(0x325e, "T-TEST");
	map_rom.emplace(0x3261, "T-SHIFT");
	map_rom.emplace(0x3267, "T-STORE");
	map_rom.emplace(0x326c, "T-EXPNENT");
	map_rom.emplace(0x326d, "X-LARGE");
	map_rom.emplace(0x3272, "NIL-BYTES");
	map_rom.emplace(0x327e, "BYTE-ZERO");
	map_rom.emplace(0x3283, "BITS-ZERO");
	map_rom.emplace(0x328a, "LESS-MASK");
	map_rom.emplace(0x3290, "IX-END");
	map_rom.emplace(0x3293, "RE-ST-TWO");
	map_rom.emplace(0x3296, "RESTK-SUB");
	map_rom.emplace(0x32b1, "RS-NRMLSE");
	map_rom.emplace(0x32b2, "RSTK-LOOP");
	map_rom.emplace(0x32bd, "RS-STORE");
    map_rom.emplace(0x335b, "CALCULATE");
    map_rom.emplace(0x335e, "GEN-ENT-1");
    map_rom.emplace(0x3362, "GEN-ENT-2");
    map_rom.emplace(0x3365, "RE-ENTRY");
    map_rom.emplace(0x336c, "SCAN-ENT");
    map_rom.emplace(0x3380, "FIRST-3D");
    map_rom.emplace(0x338c, "DOUBLE-A");
    map_rom.emplace(0x338e, "ENT-TABLE");
    map_rom.emplace(0x33a1, "DELETE");
    map_rom.emplace(0x33a2, "FP-CALC-2");
    map_rom.emplace(0x33b4, "STACK-NUM");
    map_rom.emplace(0x33c0, "MOVE-FP");
    map_rom.emplace(0x33c6, "STK-DATA");
    map_rom.emplace(0x33c8, "STK-CONST");
    map_rom.emplace(0x33de, "FORM-EXP");
    map_rom.emplace(0x33f1, "STK-ZEROS");
    map_rom.emplace(0x33f7, "SKIP-CONS");
    map_rom.emplace(0x33f8, "SKIP-NEXT");
    map_rom.emplace(0x3406, "LOC-MEM");
    map_rom.emplace(0x340f, "GET-MEM-0, ETC.");
    map_rom.emplace(0x341b, "STK-ZERO, ETC.");
    map_rom.emplace(0x342d, "ST-MEM-0, ETC.");
    map_rom.emplace(0x343c, "EXCHANGE");
    map_rom.emplace(0x343e, "SWAP-BYTE");
    map_rom.emplace(0x3449, "SERIES-06, ETC.");
    map_rom.emplace(0x3453, "G-LOOP");
    map_rom.emplace(0x346a, "ABS");
    map_rom.emplace(0x346e, "NEGATE");
    map_rom.emplace(0x3474, "NEG-TEST");
    map_rom.emplace(0x3483, "INT-CASE");
    map_rom.emplace(0x3492, "SGN");
    map_rom.emplace(0x34a5, "IN");
    map_rom.emplace(0x34ac, "PEEK");
    map_rom.emplace(0x34b3, "USR-NO");
    map_rom.emplace(0x34bc, "USR-$");
    map_rom.emplace(0x34d3, "USR-RANGE");
    map_rom.emplace(0x34e4, "USR-STACK");
    map_rom.emplace(0x34e7, "REPORT-A");
    map_rom.emplace(0x34e9, "TEST-ZERO");
    map_rom.emplace(0x34f9, "GREATER-0");
    map_rom.emplace(0x3501, "NOT");
    map_rom.emplace(0x3506, "LESS-0");
    map_rom.emplace(0x3507, "SIGN-TO-C");
    map_rom.emplace(0x350B, "FP-0/1");
    map_rom.emplace(0x351b, "OR");
    map_rom.emplace(0x3524, "NO-&-NO");
    map_rom.emplace(0x352d, "STR-&-NO");
    map_rom.emplace(0x353b, "NO-L-EQL, ETC.");
    map_rom.emplace(0x3543, "EX-OR-NOT");
    map_rom.emplace(0x354e, "NU-OR-STR");
    map_rom.emplace(0x3559, "STRINGS");
    map_rom.emplace(0x3564, "BYTE-COMP");
    map_rom.emplace(0x356b, "SECND-LOW");
    map_rom.emplace(0x3572, "BOTH-NULL");
    map_rom.emplace(0x3575, "SEC-PLUS");
    map_rom.emplace(0x3585, "FRST-LESS");
    map_rom.emplace(0x3588, "STR-TEST");
    map_rom.emplace(0x358c, "END-TESTS");
    map_rom.emplace(0x359c, "STRS-ADD");
    map_rom.emplace(0x35b7, "OTHER-STR");
    map_rom.emplace(0x35bf, "STK-PNTRS");
    map_rom.emplace(0x35c9, "CHRS");
    map_rom.emplace(0x35dc, "REPORT-B");
    map_rom.emplace(0x35de, "VAL (ALSO VAL$)");
    map_rom.emplace(0x360c, "V-RPORT-C");
    map_rom.emplace(0x361f, "STR$");
    map_rom.emplace(0x3645, "READ-IN");
    map_rom.emplace(0x365f, "R-I-STORE");
    map_rom.emplace(0x3669, "CODE");
    map_rom.emplace(0x3671, "STK-CODE");
    map_rom.emplace(0x3674, "LEN");
    map_rom.emplace(0x367a, "DEC-JR-NZ");
    map_rom.emplace(0x3686, "JUMP");
    map_rom.emplace(0x3687, "JUMP-2");
    map_rom.emplace(0x368f, "JUMP-TRUE");
    map_rom.emplace(0x369b, "END-CALC");
    map_rom.emplace(0x36a0, "N-MOD-M");
    map_rom.emplace(0x36af, "INT");
    map_rom.emplace(0x36b7, "X-NEG");
    map_rom.emplace(0x36c2, "EXIT");
    map_rom.emplace(0x36c4, "EXP");
    map_rom.emplace(0x3703, "REPORT-6");
    map_rom.emplace(0x3705, "N-NEGTV");
    map_rom.emplace(0x370c, "RESULT-OK");
    map_rom.emplace(0x370e, "RSLT-ZERO");
    map_rom.emplace(0x371a, "REPORT-A");
    map_rom.emplace(0x371c, "VALID");
    map_rom.emplace(0x373d, "GRE.8");
    map_rom.emplace(0x3783, "GET-ARGT");
    map_rom.emplace(0x37a1, "ZPLUS");
    map_rom.emplace(0x37a8, "YNEG");
    map_rom.emplace(0x37aa, "COS");
    map_rom.emplace(0x37b5, "SIN");
    map_rom.emplace(0x37b7, "C-ENT");
    map_rom.emplace(0x37da, "TAN");
    map_rom.emplace(0x37e2, "ATN");
    map_rom.emplace(0x37f8, "SMALL");
    map_rom.emplace(0x37fa, "CASES");
    map_rom.emplace(0x3833, "ASN");
    map_rom.emplace(0x3843, "ACS");
    map_rom.emplace(0x384a, "SQR");
    map_rom.emplace(0x3851, "TO-POWER");
    map_rom.emplace(0x385d, "XIS0");
    map_rom.emplace(0x386a, "ONE");
    map_rom.emplace(0x386c, "LAST");
}

const Instruction& decode_opcode(uint32_t opcode)
{
	if (map_inst.empty())
	{
		init_map_inst();
	}

	auto search_op = map_inst.find(opcode);
	if (search_op != map_inst.end())
	{
		return search_op->second;
	}

	return inv_inst;
}

bool has_rom_label(uint32_t address)
{
	if (map_rom.empty())
	{
		init_map_rom();
	}

	return map_rom.find(address) != map_rom.end();
}

const std::string& decode_rom_label(uint32_t address)
{
	if (map_rom.empty())
	{
		init_map_rom();
	}

	auto search_rom = map_rom.find(address);
	if (search_rom != map_rom.end())
	{
		return search_rom->second;
	}

	return unk_rom_addr;
}
