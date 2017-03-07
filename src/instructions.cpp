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
	assert(Operand::PC == dst);
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		//! TODO conditional check should be a member function
		if (is_cond_set(cond, state))
		{
			//! TODO need another operator implemented
			dst_elem = src_elem;
		}
	}
	
	return dst_handled && src_handled;
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
	return impl_sub(state, true /* store */, false /* update_flags */, false /* use_carry */);
}

bool Instruction::do_cp(Z80 &state)
{
	return impl_sub(state, false /* store */, true /* update_flags */, false /* use_carry */);
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
		//! TODO conditional check should be a member function
		if (is_cond_set(cond, state))
		{
			//! TODO need another operator implemented
			dst_elem = dst_elem + src_elem;
		}
	}
	
	return dst_handled && src_handled;
}

bool Instruction::do_sbc(Z80 &state)
{
	return impl_sub(state, true /* store */, true /* update_flags */, true /* use_carry */);
}

bool Instruction::do_add(Z80 &state)
{
	return impl_add(state, true /* store */, true /* update_flags */);
}

bool Instruction::do_inc(Z80 &state)
{
	return impl_add(state, true /* store */, false /* update_flags */);
}

bool Instruction::impl_add(Z80 &state, bool store, bool update_flags)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		StorageElement result = dst_elem + src_elem;

		if (update_flags)
		{
			state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
			state.af.flag(RegisterAF::Flags::AddSubtract, false);
			state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow());
			state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
			state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
			state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
		}

		if (store)
		{
			dst_elem = result;
		}
	}
	
	return dst_handled && src_handled;
	
}

bool Instruction::impl_sub(Z80 &state, bool store, bool update_flags, bool use_carry)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		StorageElement carry(use_carry && state.af.flag(RegisterAF::Flags::Carry) ? 1 : 0);
		StorageElement res_src = src_elem - carry;
		StorageElement result = dst_elem - res_src;

		if (update_flags)
		{
			state.af.flag(RegisterAF::Flags::Carry, result.is_carry() || res_src.is_carry());
			state.af.flag(RegisterAF::Flags::AddSubtract, true);
			state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow() || res_src.is_carry());
			state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half() || res_src.is_carry());
			state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
			state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
		}

		if (store)
		{
			dst_elem = result;
		}
	}
	
	return dst_handled && src_handled;
}

bool Instruction::is_cond_set(Conditional cond, Z80 &state)
{
	switch(cond)
	{
	case Conditional::ALWAYS: return true;
	case Conditional::Z:      return state.af.flag(RegisterAF::Flags::Zero);
	case Conditional::NZ:     return !state.af.flag(RegisterAF::Flags::Zero);
	case Conditional::C:      return state.af.flag(RegisterAF::Flags::Carry);
	case Conditional::NC:     return !state.af.flag(RegisterAF::Flags::Carry);
	default:
		std::cerr << "Unhandled conditional: " << static_cast<int>(cond) << std::endl;
		assert(false);
		return false;
	}
}
