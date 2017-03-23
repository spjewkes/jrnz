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
	case InstType::NOP:  return do_nop(state);
	case InstType::LD:   return do_ld(state);
	case InstType::XOR:  return do_xor(state);
	case InstType::AND:  return do_and(state);
	case InstType::JP:   return do_jp(state);
	case InstType::DI:   return do_di(state);
	case InstType::EI:   return do_ei(state);
	case InstType::OUT:  return do_out(state);
	case InstType::EX:   return do_ex(state);
	case InstType::DEC:  return do_dec(state);
	case InstType::CP:   return do_cp(state);
	case InstType::JR:   return do_jr(state);
	case InstType::SBC:  return do_sbc(state);
	case InstType::ADD:  return do_add(state);
	case InstType::INC:  return do_inc(state);
	case InstType::LDDR: return do_ld_block(state, false /* inc */);
	case InstType::LDIR: return do_ld_block(state, true /* inc */);
	case InstType::IM:   return do_im(state);
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

bool Instruction::do_ld_block(Z80 &state, bool inc)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem = src_elem;

		int adjust = ( inc ? 1 : -1 );
		state.de.set(state.de.get() + adjust);
		state.hl.set(state.hl.get() + adjust);
		state.bc.set(state.bc.get() - 1);

		state.af.flag(RegisterAF::Flags::ParityOverflow, false);

		if (state.bc.get() != 0)
		{
			state.pc.set(state.pc.get() - size);
		}
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
	state.int_on = false;
	return true;
}

bool Instruction::do_ei(Z80 &state)
{
	state.int_on = true;
	return true;
}

bool Instruction::do_im(Z80 &state)
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

bool Instruction::do_out(Z80 &state)
{
	// This is effectively a load instruction with a port being the destination
	assert(Operand::PORT == dst);
	return do_ld(state);
}

bool Instruction::do_ex(Z80 &state)
{
	bool handled = false;

	if ((Operand::UNUSED == dst) && (Operand::UNUSED == src))
	{
		state.hl.swap();
		state.bc.swap();
		state.de.swap();
		handled = true;
	}
	else
	{
		bool dst_handled = false;
		bool src_handled = false;

		StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
		StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

		if (dst_handled && src_handled)
		{
			dst_elem.swap(src_elem);
			handled = true;
		}
	}

	return handled;
}

bool Instruction::do_dec(Z80 &state)
{
	return impl_sub(state, true /* store */, false /* use_carry */, true /* is_dec */);
}

bool Instruction::do_cp(Z80 &state)
{
	return impl_sub(state, false /* store */, false /* use_carry */, false /* is_dec */);
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
	return impl_sub(state, true /* store */, true /* use_carry */, false /* is_dec */);
}

bool Instruction::do_add(Z80 &state)
{
	return impl_add(state, true /* store */, false /* is_inc */);
}

bool Instruction::do_inc(Z80 &state)
{
	return impl_add(state, true /* store */, true /* is_inc */);
}

bool Instruction::impl_add(Z80 &state, bool store, bool is_inc)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		StorageElement result = dst_elem + src_elem;

		bool update_flags = true;
		if (is_inc && dst_elem.is_16bit())
		{
			update_flags = false;
		}

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

bool Instruction::impl_sub(Z80 &state, bool store, bool use_carry, bool is_dec)
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

		bool update_flags = true;
		if (is_dec && dst_elem.is_16bit())
		{
			update_flags = false;
		}

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
