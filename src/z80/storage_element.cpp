/**
 * @brief Implementation of storage element class.
 */

#include "storage_element.hpp"

#include <cstring>

#include "common.hpp"
#include "z80.hpp"

StorageElement::StorageElement(uint8_t *_ptr, size_t _count, bool _readonly)
    : ptr(_ptr), count(_count), readonly(_readonly) {
    // A count of zero indicates an unused storage element
    assert(_count == 0 || _count == 1 || _count == 2);
    if (readonly) {
        read_only.resize(count);
        std::memcpy(&read_only[0], _ptr, _count);
        ptr = &read_only[0];
    }
}

StorageElement::StorageElement(Bus *_bus, uint16_t _addr, uint8_t *_ptr, size_t _count, bool _readonly)
    : StorageElement(_ptr, _count, _readonly) {
    if (!readonly) {
        bus = _bus;
        bus_addr = _addr;
    }
}

StorageElement::StorageElement(uint8_t v) : ptr(nullptr), count(1), readonly(true) {
    read_only.push_back(v);
    ptr = &read_only[0];
}

StorageElement::StorageElement(uint8_t lo, uint8_t hi) : ptr(nullptr), count(2), readonly(true) {
    read_only.resize(count);
    read_only[WORD_LO_BYTE_IDX] = lo;
    read_only[WORD_HI_BYTE_IDX] = hi;
    ptr = &read_only[0];
}

StorageElement::StorageElement(uint32_t v, size_t _count) : count(_count), readonly(true) {
    assert((count == 1) || (count == 2));
    read_only.resize(count);
    ptr = &read_only[0];

    switch (count) {
        case 1:
            read_only[WORD_LO_BYTE_IDX] = static_cast<uint8_t>(v);
            break;
        case 2:
            ptr[WORD_LO_BYTE_IDX] = static_cast<uint8_t>(v & 0xff);
            ptr[WORD_HI_BYTE_IDX] = static_cast<uint8_t>((v >> 8) & 0xff);
            break;
        default:
            assert(false);  // Should not get here
    }
}

StorageElement StorageElement::create_element(Z80 &state, Operand operand) {
    switch (operand) {
        case Operand::AF:
            return state.af.element();
        case Operand::BC:
            return state.bc.element();
        case Operand::DE:
            return state.de.element();
        case Operand::HL:
            return state.hl.element();
        case Operand::SP:
            return state.sp.element();
        case Operand::A:
            return state.af.element_hi();
        case Operand::B:
            return state.bc.element_hi();
        case Operand::C:
            return state.bc.element_lo();
        case Operand::D:
            return state.de.element_hi();
        case Operand::E:
            return state.de.element_lo();
        case Operand::H:
            return state.hl.element_hi();
        case Operand::L:
            return state.hl.element_lo();
        case Operand::N: {
            uint8_t byte = state.bus.read_data(state.curr_operand_pc);
            state.curr_operand_pc += 1;
            return StorageElement(byte);
        }
        case Operand::NN: {
            uint8_t lo = state.bus.read_data(state.curr_operand_pc);
            uint8_t hi = state.bus.read_data(state.curr_operand_pc + 1);
            state.curr_operand_pc += 2;
            return StorageElement(lo, hi);
        }
        case Operand::PC:
            return state.pc.element();
        case Operand::PORTC: {
            return StorageElement(state.bc.lo(), state.bc.hi());
        }
        case Operand::PORTN: {
            uint8_t byte = state.bus.read_data(state.curr_operand_pc);
            state.curr_operand_pc += 1;
            return StorageElement(byte, state.af.hi());
        }
        case Operand::I:
            return state.ir.element_hi();
        case Operand::R:
            return state.ir.element_lo();
        case Operand::IX:
            return state.ix.element();
        case Operand::IY:
            return state.iy.element();
        case Operand::IXH:
            return state.ix.element_hi();
        case Operand::IXL:
            return state.ix.element_lo();
        case Operand::IYH:
            return state.iy.element_hi();
        case Operand::IYL:
            return state.iy.element_lo();
        case Operand::indBC:
            return state.bus.read_element_from_mem(state.bc.get(), 1);
        case Operand::indDE:
            return state.bus.read_element_from_mem(state.de.get(), 1);
        case Operand::indHL:
            return state.bus.read_element_from_mem(state.hl.get(), 1);
        case Operand::indN: {
            return state.bus.read_element_from_mem(state.bus.read_addr_from_mem(state.curr_operand_pc), 1);
        }
        case Operand::indNN: {
            return state.bus.read_element_from_mem(state.bus.read_addr_from_mem(state.curr_operand_pc), 2);
        }
        case Operand::indIXN: {
            int offset = static_cast<int8_t>(state.bus.read_data(state.curr_operand_pc));
            state.curr_operand_pc += 1;
            return state.bus.read_element_from_mem(static_cast<uint16_t>(state.ix.get() + offset), 1);
        }
        case Operand::indIYN: {
            int offset = static_cast<int8_t>(state.bus.read_data(state.curr_operand_pc));
            state.curr_operand_pc += 1;
            return state.bus.read_element_from_mem(static_cast<uint16_t>(state.iy.get() + offset), 1);
        }
        case Operand::indSP:
            return state.bus.read_element_from_mem(state.sp.get(), 2);
        case Operand::ZERO:
            return StorageElement(0x00);
        case Operand::ONE:
            return StorageElement(0x01);
        case Operand::TWO:
            return StorageElement(0x02);
        case Operand::THREE:
            return StorageElement(0x03);
        case Operand::FOUR:
            return StorageElement(0x04);
        case Operand::FIVE:
            return StorageElement(0x05);
        case Operand::SIX:
            return StorageElement(0x06);
        case Operand::SEVEN:
            return StorageElement(0x07);
        case Operand::HEX_0000:
            return StorageElement(0x00, static_cast<uint8_t>(0x00));
        case Operand::HEX_0008:
            return StorageElement(0x08, static_cast<uint8_t>(0x00));
        case Operand::HEX_0010:
            return StorageElement(0x10, static_cast<uint8_t>(0x00));
        case Operand::HEX_0018:
            return StorageElement(0x18, static_cast<uint8_t>(0x00));
        case Operand::HEX_0020:
            return StorageElement(0x20, static_cast<uint8_t>(0x00));
        case Operand::HEX_0028:
            return StorageElement(0x28, static_cast<uint8_t>(0x00));
        case Operand::HEX_0030:
            return StorageElement(0x30, static_cast<uint8_t>(0x00));
        case Operand::HEX_0038:
            return StorageElement(0x38, static_cast<uint8_t>(0x00));
        case Operand::IM:
            return StorageElement(&state.int_mode, 1);
        case Operand::UNUSED:
        default:
            return StorageElement(nullptr, 0);
    }
}

StorageElement &StorageElement::operator=(const StorageElement &rhs) {
    assert(count == rhs.count);
    if ((this != &rhs) && (!readonly)) {
        for (size_t index = 0; index < count; ++index) {
            write_byte(index, rhs.ptr[index]);
        }
        flag_carry = rhs.flag_carry;
        flag_half_carry = rhs.flag_half_carry;
        flag_overflow = rhs.flag_overflow;
    }
    return *this;
}

StorageElement &StorageElement::operator=(const uint8_t rhs) {
    assert(count == 1);
    if (!readonly) {
        write_byte(0, rhs);
    }
    return *this;
}

StorageElement StorageElement::operator+(const StorageElement &rhs) {
    StorageElement result = StorageElement(to_s32() + rhs.to_s32(), count);
    result.update_carry(*this, rhs);
    result.update_carry(*this, rhs, true /* is_half */);
    result.update_overflow(*this, rhs, false /* is_sub */);
    return result;
}

StorageElement StorageElement::add_carry(const StorageElement &lhs, const StorageElement &rhs, bool carry) {
    const uint32_t lhs_val = lhs.to_u32();
    const uint32_t rhs_val = rhs.to_u32();
    const uint32_t carry_val = carry ? 1u : 0u;
    const uint32_t mask = lhs.is_8bit() ? 0xffu : 0xffffu;
    const uint32_t half_mask = lhs.is_8bit() ? 0x0fu : 0x0fffu;
    const uint32_t sign_mask = lhs.is_8bit() ? 0x80u : 0x8000u;
    const uint32_t full_result = lhs_val + rhs_val + carry_val;
    const uint32_t masked_result = full_result & mask;

    StorageElement result = StorageElement(masked_result, lhs.count);
    result.flag_carry = full_result > mask;
    result.flag_half_carry = ((lhs_val & half_mask) + (rhs_val & half_mask) + carry_val) > half_mask;
    result.flag_overflow = ((~(lhs_val ^ rhs_val) & (lhs_val ^ masked_result) & sign_mask) != 0);
    return result;
}

StorageElement StorageElement::operator-(const StorageElement &rhs) {
    StorageElement result = StorageElement(to_s32() - rhs.to_s32(), count);
    result.update_borrow(*this, rhs);
    result.update_borrow(*this, rhs, true /* is_half */);
    result.update_overflow(*this, rhs, true /* is_sub */);
    return result;
}

StorageElement StorageElement::sub_carry(const StorageElement &lhs, const StorageElement &rhs, bool carry) {
    const uint32_t lhs_val = lhs.to_u32();
    const uint32_t rhs_val = rhs.to_u32();
    const uint32_t carry_val = carry ? 1u : 0u;
    const uint32_t mask = lhs.is_8bit() ? 0xffu : 0xffffu;
    const uint32_t half_mask = lhs.is_8bit() ? 0x0fu : 0x0fffu;
    const uint32_t sign_mask = lhs.is_8bit() ? 0x80u : 0x8000u;
    const uint32_t subtrahend = rhs_val + carry_val;
    const uint32_t masked_result = (lhs_val - subtrahend) & mask;

    StorageElement result = StorageElement(masked_result, lhs.count);
    result.flag_carry = lhs_val < subtrahend;
    result.flag_half_carry = (lhs_val & half_mask) < ((rhs_val & half_mask) + carry_val);
    result.flag_overflow = (((lhs_val ^ rhs_val) & (lhs_val ^ masked_result) & sign_mask) != 0);
    return result;
}

StorageElement &StorageElement::operator^=(const StorageElement &rhs) {
    from_u32(to_u32() ^ rhs.to_u32());
    return *this;
}

StorageElement &StorageElement::operator&=(const StorageElement &rhs) {
    from_u32(to_u32() & rhs.to_u32());
    return *this;
}

StorageElement &StorageElement::operator|=(const StorageElement &rhs) {
    from_u32(to_u32() | rhs.to_u32());
    return *this;
}

bool StorageElement::operator!=(const StorageElement &rhs) const {
    if (to_u32() != rhs.to_u32()) {
        return true;
    }
    return false;
}

bool StorageElement::operator==(const StorageElement &rhs) const {
    if (to_u32() == rhs.to_u32()) {
        return true;
    }
    return false;
}

void StorageElement::swap(StorageElement &rhs) {
    assert(count == 2);
    assert(rhs.count == 2);
    uint8_t tmp_lo = ptr[WORD_LO_BYTE_IDX];
    uint8_t tmp_hi = ptr[WORD_HI_BYTE_IDX];
    uint8_t rhs_lo = rhs.ptr[WORD_LO_BYTE_IDX];
    uint8_t rhs_hi = rhs.ptr[WORD_HI_BYTE_IDX];

    write_byte(WORD_LO_BYTE_IDX, rhs_lo);
    write_byte(WORD_HI_BYTE_IDX, rhs_hi);

    rhs.write_byte(WORD_LO_BYTE_IDX, tmp_lo);
    rhs.write_byte(WORD_HI_BYTE_IDX, tmp_hi);
}

bool StorageElement::get_bit(StorageElement &rhs) {
    uint32_t tmp = to_u32();
    uint32_t mask = 1 << rhs.to_u32();
    return (tmp & mask) != 0;
}

void StorageElement::set_bit(StorageElement &rhs) {
    uint32_t tmp = to_u32();
    tmp |= 1 << rhs.to_u32();
    from_u32(tmp);
}

void StorageElement::reset_bit(StorageElement &rhs) {
    uint32_t tmp = to_u32();
    tmp &= ~(1 << rhs.to_u32()) & 0xFF;
    from_u32(tmp);
}

uint16_t StorageElement::push(Bus &bus, uint16_t addr) {
    bus.write_data(addr - 1, ptr[WORD_HI_BYTE_IDX]);
    bus.write_data(addr - 2, ptr[WORD_LO_BYTE_IDX]);
    return addr - 2;
}

uint16_t StorageElement::push(Bus &bus, uint16_t addr, uint32_t first_write_phase) {
    bus.advance_instruction_timing_to(first_write_phase);
    bus.write_observed_data(addr - 1, ptr[WORD_HI_BYTE_IDX]);
    bus.advance_instruction_timing_to(first_write_phase + 3);
    bus.write_observed_data(addr - 2, ptr[WORD_LO_BYTE_IDX]);
    return addr - 2;
}

uint16_t StorageElement::pop(Bus &bus, uint16_t addr) {
    write_byte(WORD_LO_BYTE_IDX, bus.read_data(addr));
    write_byte(WORD_HI_BYTE_IDX, bus.read_data(addr + 1));
    return addr + 2;
}

void StorageElement::rotate_right(bool rot_9bit, bool carry) {
    assert(is_8bit());

    uint32_t val = to_u32();
    uint32_t lsb = val & 0x01;

    flag_carry = (lsb == 0 ? false : true);

    val >>= 1;
    val &= 0x7f;

    if (rot_9bit) {
        val |= (carry ? 0x80 : 0x00);
    } else {
        val |= (lsb != 0 ? 0x80 : 0x00);
    }

    from_u32(val & 0xff);
}

void StorageElement::rotate_left(bool rot_9bit, bool carry) {
    assert(is_8bit());

    uint32_t val = to_u32();
    uint32_t msb = val & 0x80;

    flag_carry = (msb == 0 ? false : true);

    val <<= 1;
    val &= 0xfe;

    if (rot_9bit) {
        val |= (carry ? 0x01 : 0x00);
    } else {
        val |= (msb != 0 ? 0x01 : 0x00);
    }

    from_u32(val & 0xff);
}

void StorageElement::shift_right(bool logical) {
    assert(is_8bit());

    uint32_t val = to_u32();
    uint32_t msb = val & 0x80;
    uint32_t lsb = val & 0x01;

    flag_carry = (lsb == 0 ? false : true);

    val >>= 1;

    // Logical resets the MSB
    val &= 0x7F;

    if (!logical) {
        val |= msb;
    }

    from_u32(val & 0xff);
}

void StorageElement::shift_left(bool logical) {
    assert(is_8bit());

    uint32_t val = to_u32();
    uint32_t msb = val & 0x80;

    flag_carry = (msb == 0 ? false : true);

    val <<= 1;

    if (!logical) {
        val &= 0xfe;
    } else {
        val |= 0x01;
    }

    from_u32(val & 0xff);
}

void StorageElement::invert() {
    uint32_t val = to_u32();
    from_u32(~val);
    // if (is_8bit())
    // {
    // 	from_u32(~val & 0xff);
    // }
    // else
    // {
    // 	from_u32(~val & 0xffff);
    // }
}

uint32_t StorageElement::to_u32() const {
    uint32_t v = 0;

    switch (count) {
        case 1:
            v = static_cast<uint8_t>(*ptr);
            break;
        case 2:
            v = static_cast<uint32_t>(ptr[WORD_LO_BYTE_IDX] & 0xff);
            v |= static_cast<uint32_t>(ptr[WORD_HI_BYTE_IDX] & 0xff) << 8;
            break;
        default:
            std::cerr << "to_u32() unexpected byte count (" << count << ")\n";
            assert(false);  // Should not get here
    }

    return v;
}

uint32_t StorageElement::to_u32_half() const {
    uint32_t v = 0;

    switch (count) {
        case 1:
            v = static_cast<uint8_t>(*ptr);
            v &= 0x0f;
            break;
        case 2:
            v = static_cast<uint32_t>(ptr[WORD_LO_BYTE_IDX] & 0xff);
            v |= static_cast<uint32_t>(ptr[WORD_HI_BYTE_IDX] & 0xff) << 8;
            v &= 0xff;
            break;
        default:
            assert(false);  // Should not get here
    }

    return v;
}

int StorageElement::to_s32() const {
    int v = 0;

    switch (count) {
        case 1:
            v = static_cast<char>(*ptr);
            break;
        case 2:
            short w = static_cast<short>(ptr[WORD_LO_BYTE_IDX] & 0xff);
            w |= static_cast<short>(ptr[WORD_HI_BYTE_IDX] & 0xff) << 8;
            v = w;
    }

    return v;
}

void StorageElement::write_byte(size_t index, uint8_t v) {
    assert(index < count);
    if (readonly) {
        return;
    }
    if (bus != nullptr) {
        if (writeback_phase_valid) {
            bus->advance_instruction_timing_to(writeback_phase + static_cast<uint32_t>(index * 3));
        }
        bus->write_observed_data(static_cast<uint16_t>(bus_addr + index), v);
        return;
    }
    ptr[index] = v;
}

void StorageElement::from_u32(uint32_t v) {
    switch (count) {
        case 1:
            write_byte(0, static_cast<uint8_t>(v));
            break;
        case 2:
            write_byte(WORD_LO_BYTE_IDX, static_cast<uint8_t>(v & 0xff));
            write_byte(WORD_HI_BYTE_IDX, static_cast<uint8_t>((v >> 8) & 0xff));
            break;
        default:
            assert(false);  // Should not get here
    }
}

bool StorageElement::significant_bit(bool ishalf) const {
    uint32_t mask = 0;
    if (ishalf) {
        mask = (count == 1 ? 0x08u : 0x0800u);
    } else {
        mask = (count == 1 ? 0x80u : 0x8000u);
    }
    return (to_u32() & mask) != 0;
}

void StorageElement::update_carry(const StorageElement &op1, const StorageElement &op2, bool is_half) {
    bool res_bit = significant_bit(is_half);
    bool op1_bit = op1.significant_bit(is_half);
    bool op2_bit = op2.significant_bit(is_half);
    bool v = false;

    // If either lhs or rhs has the top bit set and the result does NOT have its top bit
    // set, then clearly there has been a carry
    if (((op1_bit && !op2_bit) || (!op1_bit && op2_bit)) && (!res_bit)) {
        v = true;
    }

    // If both significant bits are set then we will overflow on addition
    if (op1_bit && op2_bit) {
        v = true;
    }

    if (is_half) {
        flag_half_carry = v;
    } else {
        flag_carry = v;
    }
}

void StorageElement::update_borrow(const StorageElement &op1, const StorageElement &op2, bool is_half) {
    bool res_bit = significant_bit(is_half);
    bool op1_bit = op1.significant_bit(is_half);
    bool op2_bit = op2.significant_bit(is_half);
    bool v = false;

    // If the result's top bit is set but either both LHS/RHS have their top bit set or they don't
    if (((!op1_bit && !op2_bit) || (op1_bit && op2_bit)) && (res_bit)) {
        v = true;
    }

    // RHS's top bit is set but not the LHS's then borrow has occurred
    if (!op1_bit && op2_bit) {
        v = true;
    }

    if (is_half) {
        flag_half_carry = v;
    } else {
        flag_carry = v;
    }
}

void StorageElement::update_overflow(const StorageElement &op1, const StorageElement &op2, bool is_sub) {
    bool res_bit = significant_bit();
    bool op1_bit = op1.significant_bit();
    bool op2_bit = op2.significant_bit();

    if (!is_sub) {
        op2_bit = !op2_bit;
    }
    flag_overflow = (op1_bit ^ res_bit) & (op1_bit ^ op2_bit);
}

std::ostream &operator<<(std::ostream &stream, const StorageElement &e) {
    stream << std::dec << e.to_s32() << " (0x" << std::hex << e.to_u32() << ")"
           << " [" << e.count << "]" << std::dec;
    return stream;
}
