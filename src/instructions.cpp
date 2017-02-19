/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, old_pc, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, old_pc, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.load(src_elem);
	}
	
	return dst_handled && src_handled;
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

