/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

bool Instruction::execute(Z80 &state)
{
	switch (inst)
	{
	case InstType::NOP: return do_nop(state);
	case InstType::LD:  return do_ld(state);
	case InstType::XOR: return do_xor(state);
	case InstType::JP:  return do_jp(state);
	case InstType::DI:  return do_di(state);
	case InstType::OUT: return do_out(state);
	case InstType::SUB: return do_sub(state);
	case InstType::CP:  return do_cp(state);
	}

	return false;
}


bool Instruction::do_nop(Z80 &state)
{
	return true;
}

bool Instruction::do_ld(Z80 &state)
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

bool Instruction::do_xor(Z80 &state)
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

bool Instruction::do_jp(Z80 &state)
{
	// This is effectively a load instruction with PC being the destination
	assert(Operand::PC == dst);
	return do_ld(state);
}

bool Instruction::do_di(Z80 &state)
{
	state.int_on = true;
	return true;
}

bool Instruction::do_out(Z80 &state)
{
	// This is effectively a load instruction with a port being the destination
	assert(Operand::PORT == dst);
	return do_ld(state);
}

bool Instruction::do_sub(Z80 &state)
{
	bool handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, handled);

	if (handled)
	{
		dst_elem.do_dec();
	}
	
	return handled;
}

bool Instruction::do_cp(Z80 &state)
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
