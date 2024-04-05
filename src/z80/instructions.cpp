/**
 * @brief Source file implementing instructions.
 */

#include "instructions.hpp"

#include <iostream>

#include "storage_element.hpp"
#include "z80.hpp"

size_t Instruction::execute(Z80 &state) {
    StorageElement dst_elem = StorageElement::create_element(state, dst);
    StorageElement src_elem = StorageElement::create_element(state, src);

    switch (inst) {
        case InstType::NOP:
            return do_nop(state, dst_elem, src_elem);
            break;
        case InstType::LD:
            return do_ld(state, dst_elem, src_elem);
            break;
        case InstType::XOR:
            return do_xor(state, dst_elem, src_elem);
            break;
        case InstType::AND:
            return do_and(state, dst_elem, src_elem);
            break;
        case InstType::OR:
            return do_or(state, dst_elem, src_elem);
            break;
        case InstType::JP:
            return do_jp(state, dst_elem, src_elem);
            break;
        case InstType::DI:
            return do_di(state, dst_elem, src_elem);
            break;
        case InstType::EI:
            return do_ei(state, dst_elem, src_elem);
            break;
        case InstType::IN:
            return do_in(state, dst_elem, src_elem);
            break;
        case InstType::OUT:
            return do_out(state, dst_elem, src_elem);
            break;
        case InstType::EX:
            return do_ex(state, dst_elem, src_elem);
            break;
        case InstType::DEC:
            return do_dec(state, dst_elem, src_elem);
            break;
        case InstType::CP:
            return do_cp(state, dst_elem, src_elem);
            break;
        case InstType::JR:
            return do_jr(state, dst_elem, src_elem);
            break;
        case InstType::DJNZ:
            return do_djnz(state, dst_elem, src_elem);
            break;
        case InstType::SUB:
            return do_sub(state, dst_elem, src_elem);
            break;
        case InstType::SBC:
            return do_sbc(state, dst_elem, src_elem);
            break;
        case InstType::ADD:
            return do_add(state, dst_elem, src_elem);
            break;
        case InstType::ADC:
            return do_adc(state, dst_elem, src_elem);
            break;
        case InstType::INC:
            return do_inc(state, dst_elem, src_elem);
            break;
        case InstType::LDD:
            return do_ldd(state, dst_elem, src_elem);
            break;
        case InstType::LDDR:
            return do_lddr(state, dst_elem, src_elem);
            break;
        case InstType::LDI:
            return do_ldi(state, dst_elem, src_elem);
            break;
        case InstType::LDIR:
            return do_ldir(state, dst_elem, src_elem);
            break;
        case InstType::IM:
            return do_im(state, dst_elem, src_elem);
            break;
        case InstType::BIT:
            return do_bit(state, dst_elem, src_elem);
            break;
        case InstType::SET:
            return do_set(state, dst_elem, src_elem);
            break;
        case InstType::RES:
            return do_res(state, dst_elem, src_elem);
            break;
        case InstType::CALL:
            return do_call(state, dst_elem, src_elem);
            break;
        case InstType::RET:
            return do_ret(state, dst_elem, src_elem);
            break;
        case InstType::RETN:
            return do_retn(state, dst_elem, src_elem);
            break;
        case InstType::RETI:
            return do_reti(state, dst_elem, src_elem);
            break;
        case InstType::PUSH:
            return do_push(state, dst_elem, src_elem);
            break;
        case InstType::POP:
            return do_pop(state, dst_elem, src_elem);
            break;
        case InstType::RLC:
            return do_rlc(state, dst_elem, src_elem);
            break;
        case InstType::RL:
            return do_rl(state, dst_elem, src_elem);
            break;
        case InstType::RRC:
            return do_rrc(state, dst_elem, src_elem);
            break;
        case InstType::RR:
            return do_rr(state, dst_elem, src_elem);
            break;
        case InstType::SLA:
            return do_sla(state, dst_elem, src_elem);
            break;
        case InstType::SLL:
            return do_sll(state, dst_elem, src_elem);
            break;
        case InstType::SRA:
            return do_sra(state, dst_elem, src_elem);
            break;
        case InstType::SRL:
            return do_srl(state, dst_elem, src_elem);
            break;
        case InstType::RLCA:
            return do_rlca(state, dst_elem, src_elem);
            break;
        case InstType::RLA:
            return do_rla(state, dst_elem, src_elem);
            break;
        case InstType::RRCA:
            return do_rrca(state, dst_elem, src_elem);
            break;
        case InstType::RRA:
            return do_rra(state, dst_elem, src_elem);
            break;
        case InstType::RLD:
            return do_rld(state, dst_elem, src_elem);
            break;
        case InstType::SCF:
            return do_scf(state, dst_elem, src_elem);
            break;
        case InstType::CCF:
            return do_ccf(state, dst_elem, src_elem);
            break;
        case InstType::CPL:
            return do_cpl(state, dst_elem, src_elem);
            break;
        case InstType::CPI:
            return do_cpi(state, dst_elem, src_elem);
            break;
        case InstType::CPIR:
            return do_cpir(state, dst_elem, src_elem);
            break;
        case InstType::CPD:
            return do_cpd(state, dst_elem, src_elem);
            break;
        case InstType::CPDR:
            return do_cpdr(state, dst_elem, src_elem);
            break;
        case InstType::RST:
            return do_rst(state, dst_elem, src_elem);
            break;
        case InstType::HALT:
            return do_halt(state, dst_elem, src_elem);
            break;
        case InstType::DAA:
            return do_daa(state, dst_elem, src_elem);
            break;
        case InstType::NEG:
            return do_neg(state, dst_elem, src_elem);
            break;
        default:
            std::cerr << "Unknown instruction type: " << static_cast<uint32_t>(inst) << std::endl;
            assert(false);
    }

    return 0;
}

size_t Instruction::do_nop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(state);
    UNUSED(dst_elem);
    UNUSED(src_elem);

    assert(Operand::UNUSED == dst);
    assert(Operand::UNUSED == src);

    return cycles;
}

size_t Instruction::do_ld(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    if (Operand::SP == dst) {
        uint32_t new_sp;
        src_elem.get_value(new_sp);
        state.top_of_stack = static_cast<uint16_t>(new_sp);
    }
    dst_elem = src_elem;

    return cycles;
}

size_t Instruction::do_ldd(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_ld_block(state, dst_elem, src_elem, false /* inc */, false /* repeat */);
}

size_t Instruction::do_lddr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_ld_block(state, dst_elem, src_elem, false /* inc */, true /* repeat */);
}

size_t Instruction::do_ldi(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_ld_block(state, dst_elem, src_elem, true /* inc */, false /* repeat */);
}

size_t Instruction::do_ldir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_ld_block(state, dst_elem, src_elem, true /* inc */, true /* repeat */);
}

size_t Instruction::impl_ld_block(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool inc,
                                  bool repeat) {
    dst_elem = src_elem;

    int adjust = (inc ? 1 : -1);
    state.de.set(state.de.get() + adjust);
    state.hl.set(state.hl.get() + adjust);
    state.bc.set(state.bc.get() - 1);

    bool pv_new = false;
    if (state.bc.get() != 0) {
        pv_new = true;
    }

    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, pv_new);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);

    if (repeat && state.bc.get() != 0) {
        state.pc.set(state.pc.get() - size);
    }

    return cycles;
}

size_t Instruction::do_xor(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    dst_elem ^= src_elem;

    state.af.flag(RegisterAF::Flags::Carry, false);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::HalfCarry, false);
    state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

    return cycles;
}

size_t Instruction::do_and(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    dst_elem &= src_elem;

    state.af.flag(RegisterAF::Flags::Carry, false);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::HalfCarry, true);
    state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

    return cycles;
}

size_t Instruction::do_or(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    dst_elem |= src_elem;

    state.af.flag(RegisterAF::Flags::Carry, false);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::HalfCarry, false);
    state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

    return cycles;
}

size_t Instruction::do_jp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::PC == dst);

    if (is_cond_set(cond, state)) {
        dst_elem = src_elem;
    }

    return cycles;
}

size_t Instruction::do_di(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    assert(Operand::UNUSED == dst);
    assert(Operand::UNUSED == src);

    state.iff1 = false;
    state.iff2 = false;

    return cycles;
}

size_t Instruction::do_ei(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    assert(Operand::UNUSED == dst);
    assert(Operand::UNUSED == src);

    state.iff1 = true;
    state.iff2 = true;

    return cycles;
}

size_t Instruction::do_im(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(state);

    dst_elem = src_elem;

    return cycles;
}

size_t Instruction::do_in(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    // This is effectively a load instruction with a port being the source but the
    // addres is a read-only value of the 16-bit port address (the lo-byte is the
    // port number and the hi-byte will be either A or B)
    assert(Operand::PORTC == src || Operand::PORTN == src);
    assert(src_elem.is_16bit());
    uint32_t port = 0;
    src_elem.get_value(port);

    dst_elem = state.bus.read_port(port);

    if (Operand::PORTC == src) {
        // The 'in r,(c)' instruction will update status flags accordingly
        state.af.flag(RegisterAF::Flags::AddSubtract, false);
        state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
        state.af.flag(RegisterAF::Flags::HalfCarry, false);
        state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
        state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());
    }

    return cycles;
}

size_t Instruction::do_out(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    // This is effectively a load instruction with a port being the destination
    // but the addres is a read-only value of the 16-bit port address (the lo-byte
    // is the port number and the hi-byte will be either A or B)
    assert(Operand::PORTC == dst || Operand::PORTN == dst);
    assert(dst_elem.is_16bit());
    uint32_t port = 0;
    dst_elem.get_value(port);

    assert(src_elem.is_8bit());
    uint32_t src = 0;
    src_elem.get_value(src);

    state.bus.write_port(port, src);

    return cycles;
}

size_t Instruction::do_ex(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    if ((Operand::UNUSED == dst) && (Operand::UNUSED == src)) {
        state.hl.swap();
        state.bc.swap();
        state.de.swap();
    } else if ((Operand::AF == dst) && (Operand::UNUSED == src)) {
        // Special case when swapping AF with AF'
        state.af.swap();
    } else {
        dst_elem.swap(src_elem);
    }

    return cycles;
}

size_t Instruction::do_dec(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_sub(state, dst_elem, src_elem, true /* store */, false /* use_carry */, true /* is_dec */);
}

size_t Instruction::do_cp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_sub(state, dst_elem, src_elem, false /* store */, false /* use_carry */, false /* is_dec */);
}

size_t Instruction::do_jr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::PC == dst);

    if (is_cond_set(cond, state)) {
        dst_elem = dst_elem + src_elem;
    }

    return cycles;
}

size_t Instruction::do_djnz(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::PC == dst);
    assert(Conditional::NZ == cond);

    state.bc.hi(state.bc.hi() - 1);
    if (state.bc.hi() != 0) {
        dst_elem = dst_elem + src_elem;
    }

    return cycles;
}

size_t Instruction::do_call(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::PC == dst);

    if (is_cond_set(cond, state)) {
        uint16_t new_sp = dst_elem.push(state.bus, state.sp.get());
        state.sp.set(new_sp);
        dst_elem = src_elem;
    }

    return cycles;
}

size_t Instruction::do_ret(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);

    assert(Operand::PC == dst);
    assert(Operand::UNUSED == src);

    if (is_cond_set(cond, state)) {
        return impl_ret(state, dst_elem);
    }

    return cycles_not_cond;
}

size_t Instruction::do_retn(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);

    assert(Operand::PC == dst);
    assert(Operand::UNUSED == src);

    state.iff1 = state.iff2;
    return impl_ret(state, dst_elem);
}

size_t Instruction::do_reti(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);

    assert(Operand::PC == dst);
    assert(Operand::UNUSED == src);

    return impl_ret(state, dst_elem);
}

size_t Instruction::impl_ret(Z80 &state, StorageElement &pc) {
    uint16_t new_sp = pc.pop(state.bus, state.sp.get());
    state.sp.set(new_sp);

    return cycles;
}

size_t Instruction::do_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    bool is_set = dst_elem.get_bit(src_elem);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, true);
    state.af.flag(RegisterAF::Flags::Zero, !is_set);

    return cycles;
}

size_t Instruction::do_set(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_set_bit(state, dst_elem, src_elem, true /* set */);
}

size_t Instruction::do_res(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_set_bit(state, dst_elem, src_elem, false /* set */);
}

size_t Instruction::impl_set_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool set) {
    UNUSED(state);

    if (set) {
        dst_elem.set_bit(src_elem);
    } else {
        dst_elem.reset_bit(src_elem);
    }

    return cycles;
}

size_t Instruction::do_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_sub(state, dst_elem, src_elem, true /* store */, false /* use_carry */, false /* is_dec */);
}

size_t Instruction::do_sbc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_sbc(state, dst_elem, src_elem);
}

size_t Instruction::do_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_add(state, dst_elem, src_elem, true /* store */, false /* use_carry */, false /* is_inc */);
}

size_t Instruction::do_adc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_adc(state, dst_elem, src_elem);
}

size_t Instruction::do_inc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    return impl_add(state, dst_elem, src_elem, true /* store */, false /* use_carry */, true /* is_inc */);
}

size_t Instruction::impl_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry,
                             bool is_inc) {
    StorageElement carry(use_carry && state.af.flag(RegisterAF::Flags::Carry) ? 1 : 0);
    StorageElement res_src = src_elem + carry;
    StorageElement result = dst_elem + res_src;

    bool update_flags = true;
    bool reduced_flags = dst_elem.is_16bit() || Operand::IX == dst || Operand::IY == dst;
    if (is_inc && reduced_flags) {
        update_flags = false;
    }

    if (update_flags) {
        if (!is_inc) {
            // Carry flag is never updated by the inc instruction
            state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
        }

        state.af.flag(RegisterAF::Flags::AddSubtract, false);
        state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
        state.af.flag(RegisterAF::Flags::F5, result.is_half());
        if (!reduced_flags) {
            state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow());
            state.af.flag(RegisterAF::Flags::F3, result.is_overflow());
            state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
            state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
        }
    }

    if (store) {
        dst_elem = result;
    }

    return cycles;
}

size_t Instruction::impl_adc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    StorageElement result = StorageElement::add_carry(dst_elem, src_elem, state.af.flag(RegisterAF::Flags::Carry));

    state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
    state.af.flag(RegisterAF::Flags::F5, result.is_half());
    state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow());
    state.af.flag(RegisterAF::Flags::F3, result.is_overflow());
    state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, result.is_neg());

    dst_elem = result;

    return cycles;
}

size_t Instruction::impl_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry,
                             bool is_dec) {
    StorageElement carry(use_carry && state.af.flag(RegisterAF::Flags::Carry) ? 1 : 0);
    StorageElement res_src = src_elem + carry;
    StorageElement result = dst_elem - res_src;

    bool update_flags = true;
    bool reduced_flags = dst_elem.is_16bit() || Operand::IX == dst || Operand::IY == dst;
    if (is_dec && reduced_flags) {
        update_flags = false;
    }

    if (update_flags) {
        if (!is_dec) {
            // Carry flag is never updated by the dec instruction
            state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
        }

        state.af.flag(RegisterAF::Flags::AddSubtract, true);
        state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
        state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow());
        state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
        state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
    }

    if (store) {
        dst_elem = result;
    }

    return cycles;
}

size_t Instruction::impl_sbc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    StorageElement result = StorageElement::sub_carry(dst_elem, src_elem, state.af.flag(RegisterAF::Flags::Carry));

    state.af.flag(RegisterAF::Flags::Carry, result.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, true);
    state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
    state.af.flag(RegisterAF::Flags::F5, result.is_half());
    state.af.flag(RegisterAF::Flags::ParityOverflow, result.is_overflow());
    state.af.flag(RegisterAF::Flags::F3, result.is_overflow());
    state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, result.is_neg());

    dst_elem = result;

    return cycles;
}

size_t Instruction::do_push(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);

    assert(Operand::UNUSED == dst);

    size_t new_sp = src_elem.push(state.bus, state.sp.get());
    state.sp.set(new_sp);

    return cycles;
}

size_t Instruction::do_pop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);

    assert(Operand::UNUSED == src);

    size_t new_sp = dst_elem.pop(state.bus, state.sp.get());
    state.sp.set(new_sp);

    return cycles;
}

size_t Instruction::do_rlc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_rotate_left(state, dst_elem, true /* set_state */, false /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_rotate_left(state, dst_elem, true /* set_state */, true /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rrc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_rotate_right(state, dst_elem, true /* set_state */, false /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_rotate_right(state, dst_elem, true /* set_state */, true /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_sla(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_shift_left(state, dst_elem, false /* logical */);

    return cycles;
}

size_t Instruction::do_sll(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_shift_left(state, dst_elem, true /* logical */);

    return cycles;
}

size_t Instruction::do_sra(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_shift_right(state, dst_elem, false /* logical */);

    return cycles;
}

size_t Instruction::do_srl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);

    impl_shift_right(state, dst_elem, true /* logical */);

    return cycles;
}

size_t Instruction::do_rlca(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);
    assert(Operand::A == dst);

    impl_rotate_left(state, dst_elem, false /* set_state */, false /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rla(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);
    assert(Operand::A == dst);

    impl_rotate_left(state, dst_elem, false /* set_state */, true /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rrca(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);
    assert(Operand::A == dst);

    impl_rotate_right(state, dst_elem, false /* set_state */, false /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rra(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(src_elem);
    assert(Operand::UNUSED == src);
    assert(Operand::A == dst);

    impl_rotate_right(state, dst_elem, false /* set_state */, true /* rot_9bit */);

    return cycles;
}

size_t Instruction::do_rld(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    assert(Operand::UNUSED == dst);
    assert(Operand::UNUSED == src);

    StorageElement regA = StorageElement::create_element(state, Operand::A);
    StorageElement indHL = StorageElement::create_element(state, Operand::indHL);

    uint32_t regA_value;
    regA.get_value(regA_value);
    uint8_t regA_lo_nibble = regA_value & 0xf;

    uint32_t indHL_value;
    indHL.get_value(indHL_value);
    uint8_t indHL_hi_nibble = (indHL_value >> 4) & 0xf;

    StorageElement regA_new((regA_value & 0xf0) | indHL_hi_nibble);
    StorageElement indHL_new(((indHL_value << 4) & 0xf0) | regA_lo_nibble);

    regA = regA_new;
    indHL = indHL_new;

    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);

    state.af.flag(RegisterAF::Flags::ParityOverflow, regA.is_even_parity());
    state.af.flag(RegisterAF::Flags::Zero, regA.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, regA.is_neg());

    return cycles;
}

size_t Instruction::impl_rotate_left(Z80 &state, StorageElement &elem, bool set_state, bool rot_9bit) {
    elem.rotate_left(rot_9bit, state.af.flag(RegisterAF::Flags::Carry));

    state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);

    if (set_state) {
        state.af.flag(RegisterAF::Flags::ParityOverflow, elem.is_even_parity());
        state.af.flag(RegisterAF::Flags::Zero, elem.is_zero());
        state.af.flag(RegisterAF::Flags::Sign, elem.is_neg());
    }

    return cycles;
}

size_t Instruction::impl_rotate_right(Z80 &state, StorageElement &elem, bool set_state, bool rot_9bit) {
    elem.rotate_right(rot_9bit, state.af.flag(RegisterAF::Flags::Carry));

    state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);

    if (set_state) {
        state.af.flag(RegisterAF::Flags::ParityOverflow, elem.is_even_parity());
        state.af.flag(RegisterAF::Flags::Zero, elem.is_zero());
        state.af.flag(RegisterAF::Flags::Sign, elem.is_neg());
    }

    return cycles;
}

size_t Instruction::impl_shift_left(Z80 &state, StorageElement &elem, bool logical) {
    elem.shift_left(logical);

    state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::Zero, elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, elem.is_neg());

    return cycles;
}

size_t Instruction::impl_shift_right(Z80 &state, StorageElement &elem, bool logical) {
    elem.shift_right(logical);

    state.af.flag(RegisterAF::Flags::Carry, elem.is_carry());
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);
    state.af.flag(RegisterAF::Flags::ParityOverflow, elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::Zero, elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, elem.is_neg());

    return cycles;
}

size_t Instruction::do_scf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    state.af.flag(RegisterAF::Flags::Carry, true);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, false);

    return cycles;
}

size_t Instruction::do_ccf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    bool old_carry = state.af.flag(RegisterAF::Flags::Carry);

    state.af.inv_flag(RegisterAF::Flags::Carry);
    state.af.flag(RegisterAF::Flags::AddSubtract, false);
    state.af.flag(RegisterAF::Flags::HalfCarry, old_carry);

    return cycles;
}

size_t Instruction::do_cpl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    state.af.accum(~state.af.accum());

    state.af.flag(RegisterAF::Flags::AddSubtract, true);
    state.af.flag(RegisterAF::Flags::HalfCarry, true);

    return cycles;
}

size_t Instruction::do_cpi(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    return impl_cp_inc_dec(state, true /* do_inc */, false /* loop */);
}

size_t Instruction::do_cpir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    return impl_cp_inc_dec(state, true /* do_inc */, true /* loop */);
}

size_t Instruction::do_cpd(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    return impl_cp_inc_dec(state, false /* do_inc */, false /* loop */);
}

size_t Instruction::do_cpdr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    return impl_cp_inc_dec(state, false /* do_inc */, true /* loop */);
}

size_t Instruction::impl_cp_inc_dec(Z80 &state, bool do_inc, bool loop) {
    StorageElement regA = StorageElement::create_element(state, Operand::A);
    StorageElement regHL = StorageElement::create_element(state, Operand::HL);
    StorageElement regBC = StorageElement::create_element(state, Operand::BC);
    StorageElement indHL = StorageElement::create_element(state, Operand::indHL);
    StorageElement one = StorageElement::create_element(state, Operand::ONE);

    // CP (HL)
    StorageElement result = regA - indHL;
    bool set_z = false;
    if (result.is_zero()) {
        set_z = true;
    }

    if (do_inc) {
        regHL = regHL + one;
    } else {
        regHL = regHL - one;
    }

    // DEC BC
    regBC = regBC - one;
    uint32_t valueBC;
    regBC.get_value(valueBC);

    // the Z flag is set if A=(HL) before HL is increased
    state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
    state.af.flag(RegisterAF::Flags::Zero, set_z);
    state.af.flag(RegisterAF::Flags::HalfCarry, result.is_half());
    state.af.flag(RegisterAF::Flags::AddSubtract, true);
    state.af.flag(RegisterAF::Flags::ParityOverflow, (valueBC == 0 ? false : true));

    if (loop && state.bc.get() != 0 && !set_z) {
        state.pc.set(state.pc.get() - size);
        return cycles;
    }

    return cycles_not_cond;
}

size_t Instruction::do_rst(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::PC == dst);

    size_t new_sp = dst_elem.push(state.bus, state.sp.get());
    state.sp.set(new_sp);

    dst_elem = src_elem;

    return cycles;
}

size_t Instruction::do_halt(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    UNUSED(dst_elem);
    UNUSED(src_elem);

    state.halted = true;

    return cycles;
}

size_t Instruction::do_daa(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::A == src);
    assert(Operand::A == dst);

    uint32_t value;
    src_elem.get_value(value);
    uint8_t val = static_cast<uint8_t>(value & 0xff);
    bool new_carry = state.af.flag(RegisterAF::Flags::Carry);

    uint8_t sum = 0;
    if (state.af.flag(RegisterAF::Flags::HalfCarry) || (val & 0xf) > 9) {
        sum = 0x6;
    }
    if (state.af.flag(RegisterAF::Flags::Carry) || (val > 0x99)) {
        sum |= 0x60;
        new_carry = true;
    }

    if (state.af.flag(RegisterAF::Flags::AddSubtract)) {
        dst_elem = src_elem - StorageElement(sum);
    } else {
        dst_elem = src_elem + StorageElement(sum);
    }

    state.af.flag(RegisterAF::Flags::Carry, new_carry);
    state.af.flag(RegisterAF::Flags::HalfCarry, dst_elem.is_half());
    state.af.flag(RegisterAF::Flags::ParityOverflow, dst_elem.is_even_parity());
    state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

    return cycles;
}

size_t Instruction::do_neg(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem) {
    assert(Operand::A == src);
    assert(Operand::A == dst);

    uint32_t value;
    src_elem.get_value(value);

    bool new_overflow = false;
    if (value == 0x80) {
        new_overflow = true;
    }
    bool new_carry = false;
    if (value != 0x00) {
        new_carry = true;
    }

    dst_elem = StorageElement(0 - value);

    state.af.flag(RegisterAF::Flags::AddSubtract, true);
    state.af.flag(RegisterAF::Flags::Carry, new_carry);
    state.af.flag(RegisterAF::Flags::HalfCarry, dst_elem.is_half());
    state.af.flag(RegisterAF::Flags::ParityOverflow, new_overflow);
    state.af.flag(RegisterAF::Flags::Zero, dst_elem.is_zero());
    state.af.flag(RegisterAF::Flags::Sign, dst_elem.is_neg());

    return cycles;
}

bool Instruction::is_cond_set(Conditional cond, Z80 &state) {
    switch (cond) {
        case Conditional::ALWAYS:
            return true;
        case Conditional::Z:
            return state.af.flag(RegisterAF::Flags::Zero);
        case Conditional::NZ:
            return !state.af.flag(RegisterAF::Flags::Zero);
        case Conditional::C:
            return state.af.flag(RegisterAF::Flags::Carry);
        case Conditional::NC:
            return !state.af.flag(RegisterAF::Flags::Carry);
        case Conditional::M:
            return state.af.flag(RegisterAF::Flags::Sign);
        case Conditional::P:
            return !state.af.flag(RegisterAF::Flags::Sign);
        case Conditional::PE:
            return state.af.flag(RegisterAF::Flags::ParityOverflow);
        case Conditional::PO:
            return !state.af.flag(RegisterAF::Flags::ParityOverflow);
        default:
            std::cerr << "Unhandled conditional: " << static_cast<int>(cond) << std::endl;
            assert(false);
            return false;
    }
}
