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
		dst_elem.do_load(src_elem);
	}
	
	return dst_handled && src_handled;
}

bool inst_xor(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, old_pc, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, old_pc, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_xor(src_elem, state);
	}
	
	return dst_handled && src_handled;
}

bool inst_jp_nn(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, old_pc, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, old_pc, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_jmp(src_elem);
	}
	
	return dst_handled && src_handled;
}

bool inst_di(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	state.int_on = true;
	return true;
}

bool inst_out(Z80 &state, unsigned short old_pc, Operand dst, Operand src)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, old_pc, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, old_pc, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_out(src_elem);
	}
	
	return dst_handled && src_handled;
}
