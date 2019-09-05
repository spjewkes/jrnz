/**
 * @brief Implementation of z80 state class.
 */

#include "z80.hpp"
#include "decoder.hpp"

Z80::Z80(Bus &_bus) : bus(_bus)
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
		else if (int_enabled && interrupt)
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
			curr_opcode_pc = pc.get();

			uint16_t operand_offset = 0;
			const auto opcode = bus.read_opcode_from_mem(curr_opcode_pc, &operand_offset);
			assert(operand_offset != 0);
			curr_operand_pc = curr_opcode_pc + operand_offset;
			
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
		// Ignore instruction cycle count. This is typically used by the debugger to
		// allow it to single step through the code
		cycles_left = 0;
	}

	return found;
}

void Z80::reset()
{
	pc.reset();
	af.reset();
	sp.reset();

	af.set(0xffff);
	sp.set(0xffff);

	top_of_stack = 0xffff;

	int_enabled = false;
	int_mode = 0;
}
