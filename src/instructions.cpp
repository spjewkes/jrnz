/**
 * @brief Source file implementing instructions.
 */

#include <iostream>
#include "instructions.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

size_t Instruction::execute(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	switch (inst)
	{
	case InstType::NOP:  return do_nop(state, dst_elem, src_elem); break;
	case InstType::LD:   return do_ld(state, dst_elem, src_elem); break;
	case InstType::XOR:  return do_xor(state, dst_elem, src_elem); break;
	case InstType::AND:  return do_and(state, dst_elem, src_elem); break;
	case InstType::OR:   return do_or(state, dst_elem, src_elem); break;
	case InstType::JP:   return do_jp(state, dst_elem, src_elem); break;
	case InstType::DI:   return do_di(state, dst_elem, src_elem); break;
	case InstType::EI:   return do_ei(state, dst_elem, src_elem); break;
	case InstType::IN:   return do_in(state, dst_elem, src_elem); break;
	case InstType::OUT:  return do_out(state, dst_elem, src_elem); break;
	case InstType::EX:   return do_ex(state, dst_elem, src_elem); break;
	case InstType::DEC:  return do_dec(state, dst_elem, src_elem); break;
	case InstType::CP:   return do_cp(state, dst_elem, src_elem); break;
	case InstType::JR:   return do_jr(state, dst_elem, src_elem); break;
	case InstType::DJNZ: return do_djnz(state, dst_elem, src_elem); break;
	case InstType::SUB:  return do_sub(state, dst_elem, src_elem); break;
	case InstType::SBC:  return do_sbc(state, dst_elem, src_elem); break;
	case InstType::ADD:  return do_add(state, dst_elem, src_elem); break;
	case InstType::INC:  return do_inc(state, dst_elem, src_elem); break;
	case InstType::LDDR: return do_lddr(state, dst_elem, src_elem); break;
	case InstType::LDIR: return do_ldir(state, dst_elem, src_elem); break;
	case InstType::IM:   return do_im(state, dst_elem, src_elem); break;
	case InstType::BIT:  return do_bit(state, dst_elem, src_elem); break;
	case InstType::SET:  return do_set(state, dst_elem, src_elem); break;
	case InstType::RES:  return do_res(state, dst_elem, src_elem); break;
	case InstType::CALL: return do_call(state, dst_elem, src_elem); break;
	case InstType::RET:  return do_ret(state, dst_elem, src_elem); break;
	case InstType::PUSH: return do_push(state, dst_elem, src_elem); break;
	case InstType::POP:  return do_pop(state, dst_elem, src_elem); break;
	case InstType::RRA:  return do_rra(state, dst_elem, src_elem); break;
	case InstType::RRCA: return do_rrca(state, dst_elem, src_elem); break;
	case InstType::SCF:  return do_scf(state, dst_elem, src_elem); break;
	case InstType::CCF:  return do_ccf(state, dst_elem, src_elem); break;
	case InstType::RST:  return do_rst(state, dst_elem, src_elem); break;
	default:
		std::cerr << "Unknown instruction type: " << static_cast<uint32_t>(inst) << std::endl;
		assert(false);
	}

	return 0;
}


size_t Instruction::do_nop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(state);
	UNUSED(dst_elem);
	UNUSED(src_elem);

	assert(Operand::UNUSED == dst);
	assert(Operand::UNUSED == src);

	return cycles;
}

size_t Instruction::do_ld(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	if (Operand::SP == dst )
	{
		uint32_t new_sp;
		src_elem.get_value(new_sp);
		state.top_of_stack = static_cast<uint16_t>(new_sp);
	}
	dst_elem = src_elem;

	return cycles;
}

size_t Instruction::do_lddr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_ld_block(state, dst_elem, src_elem, false /* inc */);
}

size_t Instruction::do_ldir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_ld_block(state, dst_elem, src_elem, true /* inc */);
}

size_t Instruction::impl_ld_block(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool inc)
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

	return cycles;
}

size_t Instruction::do_xor(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	dst_elem ^= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

	return cycles;
}

size_t Instruction::do_and(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	dst_elem &= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, true);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

	return cycles;
}

size_t Instruction::do_or(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	dst_elem |= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

	return cycles;
}

size_t Instruction::do_jp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	assert(Operand::PC == dst);

	if (is_cond_set(cond, state))
	{
		dst_elem = src_elem;
	}

	return cycles;
}

size_t Instruction::do_di(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);
	UNUSED(src_elem);

	assert(Operand::UNUSED == dst);
	assert(Operand::UNUSED == src);

	state.int_enabled = false;

	return cycles;
}

size_t Instruction::do_ei(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);
	UNUSED(src_elem);

	assert(Operand::UNUSED == dst);
	assert(Operand::UNUSED == src);

	state.int_enabled = true;

	return cycles;
}

size_t Instruction::do_im(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(state);

	dst_elem = src_elem;

	return cycles;
}

size_t Instruction::do_in(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(state);

	// This is effectively a load instruction with a port being the source
	assert(Operand::PORT == src);
	dst_elem = src_elem;

	return cycles;
}

size_t Instruction::do_out(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(state);

	// This is effectively a load instruction with a port being the destination
	assert(Operand::PORT == dst);
	dst_elem = src_elem;

	return cycles;
}

size_t Instruction::do_ex(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	if ((Operand::UNUSED == dst) && (Operand::UNUSED == src))
	{
		state.hl.swap();
		state.bc.swap();
		state.de.swap();
	}
	else if ((Operand::AF == dst) && (Operand::UNUSED == src))
	{
		// Special case when swapping AF with AF'
		state.af.swap();
	}
	else
	{
		dst_elem.swap(src_elem);
	}

	return cycles;
}

size_t Instruction::do_dec(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_sub(state, dst_elem, src_elem, true /* store */, false /* use_carry */, true /* is_dec */);
}

size_t Instruction::do_cp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_sub(state, dst_elem, src_elem, false /* store */, false /* use_carry */, false /* is_dec */);
}

size_t Instruction::do_jr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	assert(Operand::PC == dst);

	if (is_cond_set(cond, state))
	{
		dst_elem = dst_elem + src_elem;
	}

	return cycles;
}

size_t Instruction::do_djnz(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	assert(Operand::PC == dst);
	assert(Conditional::NZ == cond);

	state.bc.hi(state.bc.hi() - 1);
	if (state.bc.hi() == 0)
	{
		dst_elem = dst_elem + src_elem;
	}

	return cycles;
}

size_t Instruction::do_call(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	assert(Operand::PC == dst);

	if (is_cond_set(cond, state))
	{
		size_t new_sp = dst_elem.push(state.bus, state.sp.get());
		state.sp.set(new_sp);
		dst_elem = src_elem;
	}

	return cycles;
}

size_t Instruction::do_ret(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(src_elem);

	assert(Operand::UNUSED == src);

	if (is_cond_set(cond, state))
	{
		size_t new_sp = dst_elem.pop(state.bus, state.sp.get());
		state.sp.set(new_sp);
	}

	return cycles;
}

size_t Instruction::do_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	bool is_set = dst_elem.get_bit(src_elem);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, true);
	state.af.flag(RegisterAF::Flags::Zero, !is_set);

	return cycles;
}

size_t Instruction::do_set(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_set_bit(state, dst_elem, src_elem, true /* set */);
}

size_t Instruction::do_res(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_set_bit(state, dst_elem, src_elem, false /* set */);
}

size_t Instruction::impl_set_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool set)
{
	UNUSED(state);

	if (set)
	{
		dst_elem.set_bit(src_elem);
	}
	else
	{
		dst_elem.reset_bit(src_elem);
	}

	return cycles;
}

size_t Instruction::do_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_sub(state, dst_elem, src_elem, true /* store */, false /* use_carry */, false /* is_dec */);
}

size_t Instruction::do_sbc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_sub(state, dst_elem, src_elem, true /* store */, true /* use_carry */, false /* is_dec */);
}

size_t Instruction::do_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_add(state, dst_elem, src_elem, true /* store */, false /* is_inc */);
}

size_t Instruction::do_inc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	return impl_add(state, dst_elem, src_elem, true /* store */, true /* is_inc */);
}

size_t Instruction::impl_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool is_inc)
{
	StorageElement result = dst_elem + src_elem;

	bool update_flags = true;
	if (is_inc && dst_elem.is_16bit())
	{
		update_flags = false;
	}

	if (update_flags)
	{
		if (!is_inc)
		{
			// Carry flag is never updated by the inc instruction
			state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
		}
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

	return cycles;
}

size_t Instruction::impl_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry, bool is_dec)
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
		if (!is_dec)
		{
			// Carry flag is never updated by the dec instruction
		}
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

	return cycles;
}

size_t Instruction::do_push(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);

	assert(Operand::UNUSED == dst);

	size_t new_sp = src_elem.push(state.bus, state.sp.get());
	state.sp.set(new_sp);

	return cycles;
}

size_t Instruction::do_pop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(src_elem);

	assert(Operand::UNUSED == src);

	size_t new_sp = dst_elem.pop(state.bus, state.sp.get());
	state.sp.set(new_sp);

	return cycles;
}

size_t Instruction::do_rra(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(src_elem);

	assert(Operand::UNUSED == src);

	dst_elem.shift_right(true, false, state.af.flag(RegisterAF::Flags::Carry));

	state.af.flag(RegisterAF::Flags::Carry, dst_elem.is_carry());
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);

	return cycles;
}

size_t Instruction::do_rrca(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(src_elem);

	assert(Operand::UNUSED == src);

	dst_elem.shift_right(true, true, state.af.flag(RegisterAF::Flags::Carry));

	state.af.flag(RegisterAF::Flags::Carry, dst_elem.is_carry());
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);

	return cycles;
}

size_t Instruction::do_scf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);
	UNUSED(src_elem);

	state.af.flag(RegisterAF::Flags::Carry, true);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);

	return cycles;
}

size_t Instruction::do_ccf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);
	UNUSED(src_elem);

	state.af.inv_flag(RegisterAF::Flags::Carry);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.inv_flag(RegisterAF::Flags::HalfCarry);

	return cycles;
}

size_t Instruction::do_rst(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem)
{
	UNUSED(dst_elem);

	assert(Operand::PC == dst);

	size_t new_sp = dst_elem.push(state.bus, state.sp.get());
	state.sp.set(new_sp);
	dst_elem = src_elem;

	return cycles;
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
