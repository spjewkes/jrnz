/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"
#include "z80.hpp"

static StorageElement get_element(Z80 &state, Operand operand, unsigned short old_pc, bool &handled)
{
	handled = true;

	switch(operand)
	{
	case BC: return state.bc.element();
	case DE: return state.de.element();
	case HL: return state.hl.element();
	case SP: return state.sp.element();
	case A:  return state.af.element_hi();
	case B:  return state.bc.element_hi();
	case C:  return state.bc.element_lo();
	case D:  return state.de.element_hi();
	case E:  return state.de.element_lo();
	case H:  return state.hl.element_hi();
	case L:  return state.hl.element_lo();
	case N:  return StorageElement(state.mem.read(old_pc+1));
	case NN: return StorageElement(state.mem.read(old_pc+1), state.mem.read(old_pc+2));
	case UNUSED:
	default:
		handled = false;
		return StorageElement(nullptr, 0);
	}
}

bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool handled = true;

	StorageElement dst_elem = get_element(state, dst, old_pc, handled);
	if (handled)
	{
		StorageElement src_elem = get_element(state, src, old_pc, handled);
		if (handled)
		{
			dst_elem.load(src_elem);
		}
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

