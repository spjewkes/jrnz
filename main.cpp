#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <iomanip>

/**
 * @brief Defines the memory of the device.
 */
class Memory
{
public:
	Memory(size_t size, char *rom_file) : m_mem(size)
		{
			if(std::ifstream rom{rom_file, std::ios::binary | std::ios::ate})
			{
				auto rom_size = rom.tellg();
				rom.seekg(0);
				rom.read(reinterpret_cast<char*>(&m_mem[0]), rom_size);
				rom.close();
			}
			else
			{
				std::cerr << "No file found called " << rom_file << std::endl;
				std::cerr << "ROM uninitialized" << std::endl;
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
				
				std::cout << std::hex << " " << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(m_mem[pos]);
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
	std::vector<unsigned char> m_mem;
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

	Memory mem(65536, argv[1]);

	mem.dump(0, 32);

	return EXIT_SUCCESS;
}
