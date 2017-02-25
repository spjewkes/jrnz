/**
 * @brief Class managing memory of the system.
 */

#ifndef __MEMORY_HPP__
#define __MEMORY_HPP__

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <cassert>
#include "storage_element.hpp"
#include "common.hpp"

/**
 * @brief Defines the memory of the device.
 */
class Memory
{
public:
	Memory(size_t size, std::string &rom_file) : mem(size)
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

	unsigned char read(size_t pos) const { return mem[pos]; }
	void write(size_t pos, unsigned char v)
		{
			if(pos >= ram_start)
			{
				mem[pos] = v;
			}
		}
	StorageElement element(size_t pos, size_t count) { return StorageElement(&mem[pos], count, (pos < ram_start)); }

	std::string dump(size_t offset, size_t size, bool add_eol=false) const
		{
			std::ostringstream output;
			const auto end = offset + size;
			const auto per_line = 16;
			auto line_count = 0;
			for(auto pos = offset; pos < end; pos++)
			{
				if(0 == line_count)
				{
					output << std::hex << "0x" << std::setw(4) << std::setfill('0') << pos << ":";
				}
				
				output << std::hex << " " << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(mem[pos]);
				line_count++;

				if(per_line == line_count)
				{
					line_count = 0;
					output << std::endl;
				}
			}
			if(add_eol && line_count != 0)
			{
				output << std::endl;
			}
			return output.str();
		}

private:
	std::vector<unsigned char> mem;
	size_t ram_start;
};

#endif // __MEMORY_HPP__
