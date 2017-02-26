/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

bool inst_nop(Z80 &state, Operand dst, Operand src)
{
	return true;
}

bool inst_ld(Z80 &state, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_copy(src_elem);
	}
	
	return dst_handled && src_handled;
}

bool inst_xor(Z80 &state, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_xor(src_elem, state);
	}
	
	return dst_handled && src_handled;
}

bool inst_jp(Z80 &state, Operand dst, Operand src)
{
	// This is effectively a load instruction with PC being the destination
	assert(Operand::PC == dst);
	return inst_ld(state, dst, src);
}

bool inst_di(Z80 &state, Operand dst, Operand src)
{
	state.int_on = true;
	return true;
}

bool inst_out(Z80 &state, Operand dst, Operand src)
{
	// This is effectively a load instruction with a port being the destination
	assert(Operand::PORT == dst);
	return inst_ld(state, dst, src);
}

bool inst_sub(Z80 &state, Operand dst, Operand src)
{
	bool handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, handled);

	if (handled)
	{
		dst_elem.do_dec();
	}
	
	return handled;
}

bool inst_cp(Z80 &state, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_subtract(src_elem, state, false);
	}
	
	return dst_handled && src_handled;
}
