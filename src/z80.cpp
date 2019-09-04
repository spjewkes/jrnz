/**
 * @brief Implementation of z80 state class.
 */

#include "z80.hpp"
#include "decoder.hpp"

Z80::Z80(Memory &memory) : mem(memory)
{
	reset();
}

bool Z80::clock(bool no_cycles)
{
	bool found = false;

	if (cycles_left == 0)
	{
		if (int_nmi)
		{
			Instruction inst {InstType::PUSH, "NMI", 1, 11, Operand::UNUSED, Operand::PC};
			cycles_left = inst.execute(*this);
			pc.set(0x66);
			int_nmi = false;
			found = true;
		}
		else if (interrupt)
		{
			switch (int_mode)
			{
			case 0:
			{
				//! TODO - we don't currently support mode 0
				assert(false);
				break;
			}
			case 1:
			{
				Instruction inst {InstType::PUSH, "INT1", 1, 13, Operand::UNUSED, Operand::PC};
				cycles_left = inst.execute(*this);
				pc.set(0x38);
				interrupt = false;
				found = true;
				break;
			}
			case 2:
			{
				//! TODO - we don't currently support mode 2
				assert(false);
				break;
			}
			}
		}
		else
		{
			const auto opcode = get_opcode(pc.get());
			std::cout << dump_instr_at_pc(pc.get()).str() << std::endl;
			const Instruction &inst = decode_opcode(opcode);
			if (inst.inst != InstType::INV)
			{
				pc.set(curr_opcode_pc + inst.size);
				cycles_left = const_cast<Instruction&>(inst).execute(*this);
				found = true;
			}
		}
	}
	else
	{
		cycles_left--;
		found = true;
	}

	if (no_cycles)
	{
		// Ignore instruction cycle count (needed by debugger)
		cycles_left = 0;
	}

	return found;
}

std::stringstream Z80::dump_instr_at_pc(uint16_t pc)
{
	std::stringstream str;

	const auto opcode = get_opcode(pc);
	const Instruction &inst = decode_opcode(opcode);
	if (inst.inst != InstType::INV)
	{
		str << std::left << std::setw(20) << mem.dump(curr_opcode_pc, inst.size);
		str << std::setw(20) << inst.name;

		if (has_rom_label(pc))
		{
			str << "Routine: " << decode_rom_label(pc);
		}
	}
	else
	{
		str << mem.dump(curr_opcode_pc, 4) << " UNKNOWN INSTRUCTION: 0x" << std::hex << opcode << std::dec;
	}
	return str;
}

void Z80::dump()
{
	std::cout << "AF: " << af << std::endl;
	std::cout << "PC: " << pc << std::endl;
	std::cout << "SP: " << sp << std::endl;
	std::cout << "BC: " << bc << std::endl;
	std::cout << "DE: " << de << std::endl;
	std::cout << "HL: " << hl << std::endl;
	std::cout << "IX: " << ix << std::endl;
	std::cout << "IY: " << iy << std::endl;
	std::cout << "IM: " << static_cast<uint32_t>(int_mode) << " ei: " << (int_enabled ? "on" : "off") << std::endl;
}

void Z80::dump_sp()
{
	assert(sp.get() <= top_of_stack);
	std::cout << "Dumping stack at SP: " << sp << std::endl;
	std::cout << mem.dump(sp.get(), top_of_stack - sp.get()) << std::endl;
	std::cout << "==== TOP OF THE STACK ====" << std::endl;
}

void Z80::reset()
{
	pc.reset();
	af.reset();
	sp.reset();

	af.set(0xffff);
	sp.set(0xffff);

	top_of_stack = 0xffff;
}

uint32_t Z80::get_opcode(uint16_t pc)
{
	curr_opcode_pc = pc;
	curr_operand_pc = curr_opcode_pc + 1;
	uint32_t opcode = mem.read(curr_opcode_pc);

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
