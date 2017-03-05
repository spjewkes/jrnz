/**
 * @brief Source file implementing instructions.
 */

#include <iostream>
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
	case InstType::AND: return do_and(state);
	case InstType::JP:  return do_jp(state);
	case InstType::DI:  return do_di(state);
	case InstType::OUT: return do_out(state);
	case InstType::DEC: return do_dec(state);
	case InstType::CP:  return do_cp(state);
	case InstType::JR:  return do_jr(state);
	case InstType::SBC: return do_sbc(state);
	case InstType::ADD: return do_add(state);
	case InstType::INC: return do_inc(state);
	default:
		std::cerr << "Unknown instruction type: " << static_cast<unsigned int>(inst) << std::endl;
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
		dst_elem = src_elem;
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
		dst_elem ^= src_elem;

		state.af.flag(RegisterAF::Flags::Carry, false);
		state.af.flag(RegisterAF::Flags::AddSubtract, false);
		state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
		state.af.flag(RegisterAF::Flags::HalfCarry, false);
		state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
		state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
	}
	
	return dst_handled && src_handled;
}

bool Instruction::do_and(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem &= src_elem;

		state.af.flag(RegisterAF::Flags::Carry, false);
		state.af.flag(RegisterAF::Flags::AddSubtract, false);
		state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
		state.af.flag(RegisterAF::Flags::HalfCarry, false);
		state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
		state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
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

bool Instruction::do_dec(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_subtract(src_elem, state, false, true, false);
	}
	
	return dst_handled && src_handled;
}

bool Instruction::do_cp(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_subtract(src_elem, state, true, false, false);
	}
	
	return dst_handled && src_handled;
}

bool Instruction::do_jr(Z80 &state)
{
	assert(Operand::PC == dst);
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_jr(src_elem, state, cond);
	}
	
	return dst_handled && src_handled;

}

bool Instruction::do_sbc(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem.do_subtract(src_elem, state, false, true, true);
	}
	
	return dst_handled && src_handled;
}

bool Instruction::do_add(Z80 &state)
{
	return impl_add(state, true /* update_state */);
}

bool Instruction::do_inc(Z80 &state)
{
	return impl_add(state, false /* update_state */);
}

bool Instruction::impl_add(Z80 &state, bool update_state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		StorageElement result = dst_elem + src_elem;

		state.af.set_borrow(dst_elem.to_u32(), src_elem.to_u32(), result.to_u32(), result.size());
		state.af.flag(RegisterAF::Flags::AddSubtract, false);
		state.af.set_overflow(dst_elem.to_u32(), src_elem.to_u32(), result.to_u32(), result.size());
		state.af.set_borrow(dst_elem.to_u32(), src_elem.to_u32(), result.to_u32(), result.size(), true);
		state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
		state.af.flag(RegisterAF::Flags::Sign, result.is_neg());

		if (update_state)
		{
			dst_elem = result;
		}
	}
	
	return dst_handled && src_handled;
	
}

bool Instruction::impl_sub(Z80 &state, bool update_state, bool store, bool use_carry)
{
	return false;
}

