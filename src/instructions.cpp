/**
 * @brief Source file implementing instructions.
 */

#include <iostream>
#include "instructions.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

void Instruction::execute(Z80 &state)
{
	switch (inst)
	{
		case InstType::NOP:  do_nop(state); break;
		case InstType::LD:   do_ld(state); break;
		case InstType::XOR:  do_xor(state); break;
		case InstType::AND:  do_and(state); break;
		case InstType::OR:   do_or(state); break;
		case InstType::JP:   do_jp(state); break;
		case InstType::DI:   do_di(state); break;
		case InstType::EI:   do_ei(state); break;
		case InstType::OUT:  do_out(state); break;
		case InstType::EX:   do_ex(state); break;
		case InstType::DEC:  do_dec(state); break;
		case InstType::CP:   do_cp(state); break;
		case InstType::JR:   do_jr(state); break;
		case InstType::DJNZ: do_djnz(state); break;
		case InstType::SUB:  do_sub(state); break;
		case InstType::SBC:  do_sbc(state); break;
		case InstType::ADD:  do_add(state); break;
		case InstType::INC:  do_inc(state); break;
		case InstType::LDDR: do_lddr(state); break;
		case InstType::LDIR: do_ldir(state); break;
		case InstType::IM:   do_im(state); break;
		case InstType::BIT:  do_bit(state); break;
		case InstType::SET:  do_set(state); break;
		case InstType::RES:  do_res(state); break;
		case InstType::CALL: do_call(state); break;
		case InstType::RET:  do_ret(state); break;
		case InstType::PUSH: do_push(state); break;
		case InstType::POP:  do_pop(state); break;
		case InstType::RRCA: do_rrca(state); break;
		case InstType::SCF:  do_scf(state); break;
		case InstType::CCF:  do_ccf(state); break;
		case InstType::RST:  do_rst(state); break;
		default:
			std::cerr << "Unknown instruction type: " << static_cast<unsigned int>(inst) << std::endl;
			assert(false);
	}
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

bool Instruction::do_lddr(Z80 &state)
{
	return impl_ld_block(state, false /* inc */);
}

bool Instruction::do_ldir(Z80 &state)
{
	return impl_ld_block(state, true /* inc */);
}

bool Instruction::impl_ld_block(Z80 &state, bool inc)
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

bool Instruction::do_or(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		dst_elem |= src_elem;

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
		if (is_cond_set(cond, state))
		{
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
		if (is_cond_set(cond, state))
		{
			dst_elem = dst_elem + src_elem;
		}
	}

	return dst_handled && src_handled;
}

bool Instruction::do_djnz(Z80 &state)
{
	assert(Operand::PC == dst);
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		assert(Conditional::NZ == cond);

		state.bc.hi(state.bc.hi() - 1);
		if (state.bc.hi() == 0)
		{
			dst_elem = dst_elem + src_elem;
		}
	}

	return dst_handled && src_handled;
}

bool Instruction::do_call(Z80 &state)
{
	assert(Operand::PC == dst);
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		if (is_cond_set(cond, state))
		{
			size_t new_sp = dst_elem.push(state.mem, state.sp.get());
			state.sp.set(new_sp);
			dst_elem = src_elem;
		}
	}

	return dst_handled && src_handled;
}

bool Instruction::do_ret(Z80 &state)
{
	assert(Operand::UNUSED == src);
	bool handled = false;

	StorageElement elem = StorageElement::create_element(state, dst, handled);

	if (handled)
	{
		if (is_cond_set(cond, state))
		{
			size_t new_sp = elem.pop(state.mem, state.sp.get());
			state.sp.set(new_sp);
		}
	}

	return handled;
}

bool Instruction::do_bit(Z80 &state)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		bool is_set = dst_elem.get_bit(src_elem);
		state.af.flag(RegisterAF::Flags::AddSubtract, false);
		state.af.flag(RegisterAF::Flags::HalfCarry, true);
		state.af.flag(RegisterAF::Flags::Zero, is_set);
	}

	return dst_handled && src_handled;
}

bool Instruction::do_set(Z80 &state)
{
	return impl_set_bit(state, true /* set */);
}

bool Instruction::do_res(Z80 &state)
{
	return impl_set_bit(state, false /* set */);
}

bool Instruction::impl_set_bit(Z80 &state, bool set)
{
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		if (set)
		{
			dst_elem.set_bit(src_elem);
		}
		else
		{
			dst_elem.reset_bit(src_elem);
		}
	}

	return dst_handled && src_handled;
}

bool Instruction::do_sub(Z80 &state)
{
	return impl_sub(state, true /* store */, false /* use_carry */, false /* is_dec */);
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

bool Instruction::do_push(Z80 &state)
{
	assert(Operand::UNUSED == dst);
	bool src_handled = false;

	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (src_handled)
	{
		size_t new_sp = src_elem.push(state.mem, state.sp.get());
		state.sp.set(new_sp);
	}

	return src_handled;
}

bool Instruction::do_pop(Z80 &state)
{
	assert(Operand::UNUSED == src);
	bool dst_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);

	if (dst_handled)
	{
		size_t new_sp = dst_elem.pop(state.mem, state.sp.get());
		state.sp.set(new_sp);
	}

	return dst_handled;
}

bool Instruction::do_rrca(Z80 &state)
{
	assert(Operand::UNUSED == src);
	bool handled = false;

	StorageElement elem = StorageElement::create_element(state, dst, handled);

	if (handled)
	{
		elem.shift_right(true);

		state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
		state.af.flag(RegisterAF::Flags::AddSubtract, false);
		state.af.flag(RegisterAF::Flags::HalfCarry, false);
	}

	return handled;
}

bool Instruction::do_scf(Z80 &state)
{
	state.af.flag(RegisterAF::Flags::Carry, true);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);

	return true;
}

bool Instruction::do_ccf(Z80 &state)
{
	state.af.inv_flag(RegisterAF::Flags::Carry);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.inv_flag(RegisterAF::Flags::HalfCarry);

	return true;
}

bool Instruction::do_rst(Z80 &state)
{
	assert(Operand::PC == dst);
	bool dst_handled = false;
	bool src_handled = false;

	StorageElement dst_elem = StorageElement::create_element(state, dst, dst_handled);
	StorageElement src_elem = StorageElement::create_element(state, src, src_handled);

	if (dst_handled && src_handled)
	{
		size_t new_sp = dst_elem.push(state.mem, state.sp.get());
		state.sp.set(new_sp);
		dst_elem = src_elem;
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
	case Conditional::M:      return state.af.flag(RegisterAF::Flags::AddSubtract);
	case Conditional::P:      return !state.af.flag(RegisterAF::Flags::AddSubtract);
	case Conditional::PE:     return state.af.flag(RegisterAF::Flags::ParityOverflow);
	case Conditional::PO:     return !state.af.flag(RegisterAF::Flags::ParityOverflow);
	default:
		std::cerr << "Unhandled conditional: " << static_cast<int>(cond) << std::endl;
		assert(false);
		return false;
	}
}
