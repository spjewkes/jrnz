/**
 * @brief Class managing memory/data bus of the system.
 */

#ifndef __BUS_HPP__
#define __BUS_HPP__

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cassert>
#include <cstdint>
#include "storage_element.hpp"
#include "common.hpp"

/**
 * @brief Defines the memory/data bus of the device.
 */
class Bus
{
public:
	Bus(size_t size) : mem(size) {}
	virtual ~Bus() {}

	void load_rom(std::string &rom_file)
		{
			if(std::ifstream rom{rom_file, std::ios::binary | std::ios::ate})
			{
				auto rom_size = rom.tellg();
				ram_start = rom_size;
				rom.seekg(0);
				rom.read(reinterpret_cast<char*>(&mem[0]), rom_size);
				rom.close();
			}
			else
			{
				std::cerr << "No file found called " << rom_file << std::endl;
				std::cerr << "ROM uninitialized" << std::endl;
			}
		}

	uint8_t &operator[](uint16_t addr)
		{
			return mem[addr];
		}

	uint8_t read_data(uint16_t addr) const { return mem[addr]; }

	void write_data(uint16_t addr, uint8_t v)
		{
			if(addr >= ram_start)
			{
				mem[addr] = v;
			}
		}

	uint16_t read_addr_from_mem(uint16_t addr) const
		{
			uint16_t ret_addr = mem[addr];
			ret_addr |= mem[addr+1] << 8;
			return ret_addr;
		}
	void write_addr_to_mem(uint16_t addr, uint16_t addr_to_write)
		{
			write_data(addr, addr_to_write & 0xff);
			write_data(addr + 1, (addr_to_write >> 8) & 0xff);
		}

	StorageElement read_element_from_mem(uint16_t addr, size_t count) { return StorageElement(&mem[addr], count, (addr < ram_start)); }

	uint32_t read_opcode_from_mem(uint16_t addr, uint16_t* operand_offset = nullptr)
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

	void clock()
		{
			// Not actively used at the moment but may be useful for debugging
		}
	
private:
	std::vector<uint8_t> mem;
	uint16_t ram_start;
};

#endif // __BUS_HPP__
