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
	void hi(unsigned char v)
		{
			reg &= 0x00ff;
			reg |= v << 8;
		}
	unsigned char hi() const { return static_cast<unsigned char>((reg >> 8) & 0xff); }
	void lo(unsigned char v)
		{
			reg &= 0xff00;
			reg |= v;
		}
	unsigned char lo() const { return static_cast<unsigned char>(reg & 0xff); }
	void set(unsigned short v) { reg = v; }
	unsigned short get() const { return reg; }
	void swap() { std::swap(reg, alt_reg); }

private:
	unsigned short reg = 0;
	unsigned short alt_reg = 0;
};

class Z80;

enum Operand
{
	BC,
	DE,
	HL,
	SP,
	A,
	B,
	C,
	D,
	E,
	H,
	L,
	N,
	NN,
	UNUSED
};

typedef bool(*inst_fn)(Z80&,unsigned short, Operand, Operand);

typedef struct Instruction
{
public:
	std::string name;
	unsigned int size;
	unsigned int cycles;
	inst_fn func;
	Operand dst;
	Operand src;
} Instruction;

bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_xor(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_jp_nn(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_di(Z80 &state, unsigned short old_pc, Operand dst, Operand src);

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
	Register pc;
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

bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool handled = true;
	
	switch(dst)
	{
	case Operand::DE:
	{
		state.de.lo(state.mem.read(old_pc+1));
		state.de.hi(state.mem.read(old_pc+2));
		break;
	}
	case Operand::B:
	{
		state.bc.lo(state.af.lo());
		break;
	}
	default:
		handled = false;
		break;
	}

	return handled;
}

bool inst_xor(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool handled = false;

	switch(state.mem.read(old_pc))
	{
	case 0xaf:
	{
		unsigned char a = state.af.lo();
		state.af.lo(a^a);
		handled = true;
	}
	}

	return handled;
}

bool inst_jp_nn(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool handled = false;

	switch(state.mem.read(old_pc))
	{
	case 0xc3:
	{
		state.pc.lo(state.mem.read(old_pc+1));
		state.pc.hi(state.mem.read(old_pc+2));
		handled = true;
	}
	}

	return handled;
}

bool inst_di(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	state.int_on = true;
	return true;
}

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
