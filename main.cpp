#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <iomanip>

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

	void dump(size_t offset, size_t size) const
		{
			const auto end = offset + size;
			const auto per_line = 16;
			auto line_count = 0;
			for(auto pos = offset; pos < end; pos++)
			{
				if(0 == line_count)
				{
					std::cout << std::hex << "0x" << std::setw(4) << std::setfill('0') << pos << ":";
				}
				
				std::cout << std::hex << " " << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(mem[pos]);
				line_count++;

				if(per_line == line_count)
				{
					line_count = 0;
					std::cout << std::endl;
				}
			}
			if(line_count != 0)
			{
				std::cout << std::endl;
			}
		}

private:
	std::vector<unsigned char> mem;
	size_t ram_start;
};

/**
 * @brief Class describing a 16-bit register.
 */
class Register
{
public:
	void set_hi(unsigned char v)
		{
			reg &= 0x0f;
			reg |= v << 8;
		}
	unsigned char get_hi() const { return static_cast<unsigned char>((reg >> 8) & 0xf); }
	void set_lo(unsigned char v)
		{
			reg &= 0xf0;
			reg |= v;
		}
	unsigned char get_lo() const { return static_cast<unsigned char>(reg & 0xf); }
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
			map_inst.emplace(0xf3, std::string("di"));
		}

	unsigned short i = { 0 };
	unsigned short pc = { 0 };
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

			unsigned char v = mem.read(pc);
			auto search = map_inst.find(v);
			if(search != map_inst.end())
			{
				std::cout << map_inst[v] << std::endl;
				found = true;
			}
			else
			{
				std::cout << "Unknown byte: 0x" << std::setw(2) << std::hex << static_cast<unsigned int>(v) << std::endl;
			}
			pc++;
			return found;
		}

private:
	std::map<unsigned char, const std::string> map_inst;
};

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

	if(argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << " <rom image>" << std::endl;
		return EXIT_FAILURE;
	}

	std::string rom_file(argv[1]);
	Z80 state(65536, rom_file);

	while(state.clock())
	{
	}

	return EXIT_SUCCESS;
}
