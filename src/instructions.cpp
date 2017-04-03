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


void Instruction::do_nop(Z80 &state)
{
}

void Instruction::do_ld(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	dst_elem = src_elem;
}

void Instruction::do_lddr(Z80 &state)
{
	impl_ld_block(state, false /* inc */);
}

void Instruction::do_ldir(Z80 &state)
{
	impl_ld_block(state, true /* inc */);
}

void Instruction::impl_ld_block(Z80 &state, bool inc)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

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

void Instruction::do_xor(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	dst_elem ^= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
}

void Instruction::do_and(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	dst_elem &= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
}

void Instruction::do_or(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	dst_elem |= src_elem;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
}

void Instruction::do_jp(Z80 &state)
{
	assert(Operand::PC == dst);

	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	if (is_cond_set(cond, state))
	{
		dst_elem = src_elem;
	}
}

void Instruction::do_di(Z80 &state)
{
	state.int_on = false;
}

void Instruction::do_ei(Z80 &state)
{
	state.int_on = true;
}

void Instruction::do_im(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	dst_elem = src_elem;
}

void Instruction::do_out(Z80 &state)
{
	// This is effectively a load instruction with a port being the destination
	assert(Operand::PORT == dst);
	do_ld(state);
}

void Instruction::do_ex(Z80 &state)
{
	if ((Operand::UNUSED == dst) && (Operand::UNUSED == src))
	{
		state.hl.swap();
		state.bc.swap();
		state.de.swap();
	}
	else
	{
		StorageElement dst_elem = StorageElement::create_element(state, dst);
		StorageElement src_elem = StorageElement::create_element(state, src);

		dst_elem.swap(src_elem);
	}
}

void Instruction::do_dec(Z80 &state)
{
	impl_sub(state, true /* store */, false /* use_carry */, true /* is_dec */);
}

void Instruction::do_cp(Z80 &state)
{
	impl_sub(state, false /* store */, false /* use_carry */, false /* is_dec */);
}

void Instruction::do_jr(Z80 &state)
{
	assert(Operand::PC == dst);

	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	if (is_cond_set(cond, state))
	{
		dst_elem = dst_elem + src_elem;
	}
}

void Instruction::do_djnz(Z80 &state)
{
	assert(Operand::PC == dst);

	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	assert(Conditional::NZ == cond);

	state.bc.hi(state.bc.hi() - 1);
	if (state.bc.hi() == 0)
	{
		dst_elem = dst_elem + src_elem;
	}
}

void Instruction::do_call(Z80 &state)
{
	assert(Operand::PC == dst);

	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	if (is_cond_set(cond, state))
	{
		size_t new_sp = dst_elem.push(state.mem, state.sp.get());
		state.sp.set(new_sp);
		dst_elem = src_elem;
	}
}

void Instruction::do_ret(Z80 &state)
{
	assert(Operand::UNUSED == src);

	StorageElement elem = StorageElement::create_element(state, dst);

	if (is_cond_set(cond, state))
	{
		size_t new_sp = elem.pop(state.mem, state.sp.get());
		state.sp.set(new_sp);
	}
}

void Instruction::do_bit(Z80 &state)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	bool is_set = dst_elem.get_bit(src_elem);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, true);
	state.af.flag(RegisterAF::Flags::Zero, is_set);
}

void Instruction::do_set(Z80 &state)
{
	impl_set_bit(state, true /* set */);
}

void Instruction::do_res(Z80 &state)
{
	impl_set_bit(state, false /* set */);
}

void Instruction::impl_set_bit(Z80 &state, bool set)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	if (set)
	{
		dst_elem.set_bit(src_elem);
	}
	else
	{
		dst_elem.reset_bit(src_elem);
	}
}

void Instruction::do_sub(Z80 &state)
{
	impl_sub(state, true /* store */, false /* use_carry */, false /* is_dec */);
}

void Instruction::do_sbc(Z80 &state)
{
	impl_sub(state, true /* store */, true /* use_carry */, false /* is_dec */);
}

void Instruction::do_add(Z80 &state)
{
	impl_add(state, true /* store */, false /* is_inc */);
}

void Instruction::do_inc(Z80 &state)
{
	impl_add(state, true /* store */, true /* is_inc */);
}

void Instruction::impl_add(Z80 &state, bool store, bool is_inc)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

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

void Instruction::impl_sub(Z80 &state, bool store, bool use_carry, bool is_dec)
{
	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

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

void Instruction::do_push(Z80 &state)
{
	assert(Operand::UNUSED == dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	size_t new_sp = src_elem.push(state.mem, state.sp.get());
	state.sp.set(new_sp);
}

void Instruction::do_pop(Z80 &state)
{
	assert(Operand::UNUSED == src);
	StorageElement dst_elem = StorageElement::create_element(state, dst);

	size_t new_sp = dst_elem.pop(state.mem, state.sp.get());
	state.sp.set(new_sp);
}

void Instruction::do_rrca(Z80 &state)
{
	assert(Operand::UNUSED == src);
	StorageElement elem = StorageElement::create_element(state, dst);

	elem.shift_right(true);

	state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
}

void Instruction::do_scf(Z80 &state)
{
	state.af.flag(RegisterAF::Flags::Carry, true);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
}

void Instruction::do_ccf(Z80 &state)
{
	state.af.inv_flag(RegisterAF::Flags::Carry);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.inv_flag(RegisterAF::Flags::HalfCarry);
}

void Instruction::do_rst(Z80 &state)
{
	assert(Operand::PC == dst);

	StorageElement dst_elem = StorageElement::create_element(state, dst);
	StorageElement src_elem = StorageElement::create_element(state, src);

	size_t new_sp = dst_elem.push(state.mem, state.sp.get());
	state.sp.set(new_sp);
	dst_elem = src_elem;
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
