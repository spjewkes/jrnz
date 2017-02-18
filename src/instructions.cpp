/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"
#include "registers.hpp"

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
