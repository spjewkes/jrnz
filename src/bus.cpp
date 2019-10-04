/**
 * Implementation of the memory/data bus system.
 */

#include "bus.hpp"
#include "z80.hpp"

void Bus::load_rom(std::string &rom_file)
{
	if (std::ifstream rom{rom_file, std::ios::binary | std::ios::ate})
	{
		auto rom_size = rom.tellg();
		ram_start = rom_size;
		rom.seekg(0);
		rom.read(reinterpret_cast<char*>(&mem[0]), rom_size);
		rom.close();
	}
	else
	{
		std::cerr << "No ROM file found called " << rom_file << std::endl;
		std::cerr << "ROM uninitialized" << std::endl;
	}
}

static uint8_t get_next_byte(std::ifstream &stream)
{
	char ch;
	stream.get(ch);
	return static_cast<uint8_t>(ch);
}

void Bus::load_snapshot(std::string &sna_file, Z80 &state)
{
	if (std::ifstream sna{sna_file, std::ios::binary | std::ios::ate})
	{
		auto file_size = sna.tellg();
		if (file_size != 49179)
		{
			std::cerr << "SNA file size is " << file_size << std::endl;
			std::cerr << "Expected 49179 bytes.\n";
			return;
		}

		sna.seekg(0);

		// 0x00 - I
		state.ir.hi(get_next_byte(sna));
		
		// 0x01 - HL'
		state.hl.lo(get_next_byte(sna));
		state.hl.hi(get_next_byte(sna));
		state.hl.swap();

		// 0x03 - DE'
		state.de.lo(get_next_byte(sna));
		state.de.hi(get_next_byte(sna));
		state.de.swap();

		// 0x05 - BC'
		state.bc.lo(get_next_byte(sna));
		state.bc.hi(get_next_byte(sna));
		state.bc.swap();

		// 0x07 - AF'
		state.af.lo(get_next_byte(sna));
		state.af.hi(get_next_byte(sna));
		state.af.swap();

		// 0x09 - HL
		state.hl.lo(get_next_byte(sna));
		state.hl.hi(get_next_byte(sna));

		// 0x0b - DE
		state.de.lo(get_next_byte(sna));
		state.de.hi(get_next_byte(sna));

		// 0x0d - BC
		state.bc.lo(get_next_byte(sna));
		state.bc.hi(get_next_byte(sna));

		// 0x0f - IY
		state.iy.lo(get_next_byte(sna));
		state.iy.hi(get_next_byte(sna));

		// 0x11 - IX
		state.ix.lo(get_next_byte(sna));
		state.ix.hi(get_next_byte(sna));

		// 0x13 - IFF2
		state.iff2 = (get_next_byte(sna) & 0x4 ? true : false);

		// 0x14 - R
		state.ir.lo(get_next_byte(sna));

		// 0x15 - AF
		state.af.lo(get_next_byte(sna));
		state.af.hi(get_next_byte(sna));

		// 0x17 - SP
		state.sp.lo(get_next_byte(sna));
		state.sp.hi(get_next_byte(sna));

		// 0x19 - interrupt mode: 0, 1 or 2
		state.int_mode = get_next_byte(sna);
		assert(state.int_mode == 0 || state.int_mode == 1 || state.int_mode == 2);

		// 0x1a - border colour
		get_next_byte(sna);
		//! TODO - ignore for now

		// Renaming file is the 48k of RAM
		sna.read(reinterpret_cast<char*>(&mem[16384]), 49152);
		sna.close();

		// Now execute a RETN instruction
		Instruction inst{InstType::RETN, "retn", 2, 14, Operand::PC};
		inst.execute(state);
	}
	else
	{
		std::cerr << "No SNA file found called " << sna_file << std::endl;
		std::cerr << "SNA failed to load" << std::endl;
	}
}

uint8_t Bus::read_port(uint16_t addr) const
{
	// The only port we care about is 0xfe. More specifically for now we just check that
	// the lowest bit is not set. The bits are set as follows:
	// 0-4 : keyboard
	// 5   : unused
	// 6   : ear
	// 7   : unused
	//! ear is currently not handled

	if (addr % 2 == 0)
	{
		uint8_t half_rows = (addr & 0xff00) >> 8;
		return get_keyboard_state(half_rows);
	}

	return 0;
}

void Bus::write_port(uint16_t addr, uint8_t v)
{
	if (addr == 254)
	{
		port_254 = v;
	}
}

uint32_t Bus::read_opcode_from_mem(uint16_t addr, uint16_t* operand_offset)
{
	uint16_t offset = 1;
	uint32_t opcode = mem[addr];

	// Handled extended instructions
	switch(opcode)
	{
	case 0xed:
	case 0xcb:
	case 0xdd:
	case 0xfd:
	{
		opcode = (opcode << 8) | mem[addr + 1];
		offset++;

		// Handle IX and IY bit instructions, the opcode comes after
		// the displacement byte:
		// 0xddcb <displacement byte> <opcode>
		// oxfdcb <displacement byte> <opcode>
		// Make sure the operand is not in the returned opcode
		switch(opcode)
		{
		case 0xddcb:
		case 0xfdcb:
			opcode = (opcode << 8) | mem[addr + 3];
		}
	}
	}

	if (operand_offset != nullptr)
	{
		*operand_offset = offset;
	}

	return opcode;
}
