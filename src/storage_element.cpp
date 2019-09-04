/**
 * @brief Implementation of storage element class.
 */

#include <cstring>
#include "common.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

StorageElement::StorageElement(uint8_t *_ptr, size_t _count, bool _readonly) : ptr(_ptr), count(_count), readonly(_readonly)
{
	if (readonly)
	{
		read_only.resize(count);
		std::memcpy(&read_only[0], _ptr, _count);
	}
}

StorageElement::StorageElement(uint8_t v) : ptr(nullptr), count(1), readonly(true)
{
	read_only.push_back(v);
	ptr = &read_only[0];
}

StorageElement::StorageElement(uint8_t lo, uint8_t hi) : ptr(nullptr), count(2), readonly(true)
{
	read_only.resize(count);
	read_only[WORD_LO_BYTE_IDX] = lo;
	read_only[WORD_HI_BYTE_IDX] = hi;
	ptr = &read_only[0];
}

StorageElement::StorageElement(uint32_t v, size_t _count) : count(_count), readonly(true)
{
	assert((count == 1) || (count == 2));
	read_only.resize(count);
	ptr = &read_only[0];

	switch (count)
	{
	case 1:
		read_only[WORD_LO_BYTE_IDX] = static_cast<uint8_t>(v);
		break;
	case 2:
		ptr[WORD_LO_BYTE_IDX] = static_cast<uint8_t>(v & 0xff);
		ptr[WORD_HI_BYTE_IDX] = static_cast<uint8_t>((v >> 8) & 0xff);
		break;
	default:
		assert(false); // Should not get here
	}
}

StorageElement StorageElement::create_element(Z80 &state, Operand operand)
{
	switch(operand)
	{
	case Operand::AF:       return state.af.element();
	case Operand::BC:       return state.bc.element();
	case Operand::DE:       return state.de.element();
	case Operand::HL:       return state.hl.element();
	case Operand::SP:       return state.sp.element();
	case Operand::A:        return state.af.element_hi();
	case Operand::B:        return state.bc.element_hi();
	case Operand::C:        return state.bc.element_lo();
	case Operand::D:        return state.de.element_hi();
	case Operand::E:        return state.de.element_lo();
	case Operand::H:        return state.hl.element_hi();
	case Operand::L:        return state.hl.element_lo();
	case Operand::N:
	{
		uint8_t byte = state.bus.read_data(state.curr_operand_pc);
		state.curr_operand_pc += 1;
		return StorageElement(byte);
	}
	case Operand::NN:
	{
		uint8_t lo = state.bus.read_data(state.curr_operand_pc);
		uint8_t hi = state.bus.read_data(state.curr_operand_pc+1);
		state.curr_operand_pc += 2;
		return StorageElement(lo, hi);
	}
	case Operand::PC:       return state.pc.element();
	case Operand::PORT:
	{
		uint8_t byte = state.bus.read_data(state.curr_operand_pc);
		state.curr_operand_pc += 1;
		return state.ports.element(byte);
	}
	case Operand::I:        return state.ir.element_hi();
	case Operand::R:        return state.ir.element_lo();
	case Operand::IX:       return state.ix.element();
	case Operand::IY:       return state.iy.element();
	case Operand::indBC:    return StorageElement(&state.bus[state.bc.get()],1);
	case Operand::indDE:    return StorageElement(&state.bus[state.de.get()],1);
	case Operand::indHL:    return StorageElement(&state.bus[state.hl.get()],1);
	case Operand::indN:
	{
		size_t bytes = state.bus.read_addr_from_mem(state.curr_operand_pc);
		state.curr_operand_pc += 1;
		return state.bus.read_element_from_mem(bytes, 1);
	}
	case Operand::indNN:
	{
		size_t bytes = state.bus.read_addr_from_mem(state.curr_operand_pc);
		state.curr_operand_pc += 2;
		return state.bus.read_element_from_mem(bytes, 2);
	}
	case Operand::indIXN:
	{
		uint8_t *ptr = &state.bus[state.ix.get()] + state.bus.read_data(state.curr_operand_pc);
		state.curr_operand_pc += 1;
		return StorageElement(ptr, 1);
	}
	case Operand::indIYN:
	{
		uint8_t *ptr = &state.bus[state.iy.get()] + state.bus.read_data(state.curr_operand_pc);
		state.curr_operand_pc += 1;
		return StorageElement(ptr, 1);
	}
	case Operand::indSP:    return StorageElement(&state.bus[state.sp.get()],2);
	case Operand::ZERO:     return StorageElement(0x00);
	case Operand::ONE:      return StorageElement(0x01);
	case Operand::TWO:      return StorageElement(0x02);
	case Operand::THREE:    return StorageElement(0x03);
	case Operand::FOUR:     return StorageElement(0x04);
	case Operand::FIVE:     return StorageElement(0x05);
	case Operand::SIX:      return StorageElement(0x06);
	case Operand::SEVEN:    return StorageElement(0x07);
	case Operand::HEX_0008: return StorageElement(0x08, static_cast<uint8_t>(0x00));
	case Operand::HEX_0010: return StorageElement(0x10, static_cast<uint8_t>(0x00));
	case Operand::HEX_0018: return StorageElement(0x18, static_cast<uint8_t>(0x00));
	case Operand::HEX_0020: return StorageElement(0x20, static_cast<uint8_t>(0x00));
	case Operand::HEX_0028: return StorageElement(0x28, static_cast<uint8_t>(0x00));
	case Operand::HEX_0030: return StorageElement(0x30, static_cast<uint8_t>(0x00));
	case Operand::HEX_0038: return StorageElement(0x38, static_cast<uint8_t>(0x00));
	case Operand::IM:       return StorageElement(&state.int_mode,1);
	case Operand::UNUSED:
	default:
		return StorageElement(nullptr, 0);
	}
}

StorageElement &StorageElement::operator=(const StorageElement &rhs)
{
	assert(count == rhs.count);
	if ((this != &rhs) && (!readonly))
	{
		std::memcpy(ptr, rhs.ptr, count);
		flag_carry = rhs.flag_carry;
		flag_half_carry = rhs.flag_half_carry;
		flag_overflow = rhs.flag_overflow;
	}
	return *this;
}

StorageElement StorageElement::operator+(const StorageElement &rhs)
{
	StorageElement result = StorageElement(to_s32() + rhs.to_s32(), count);
	result.update_carry(*this, rhs);
	result.update_carry(*this, rhs, true /* is_half */);
	result.update_overflow(*this, rhs);
	return result;
}

StorageElement StorageElement::operator-(const StorageElement &rhs)
{
	StorageElement result = StorageElement(to_s32() - rhs.to_s32(), count);
	result.update_borrow(*this, rhs);
	result.update_borrow(*this, rhs, true /* is_half */);
	result.update_overflow(*this, rhs);
	return result;
}

StorageElement &StorageElement::operator^=(const StorageElement &rhs)
{
	from_u32(to_u32() ^ rhs.to_u32());
	return *this;
}

StorageElement &StorageElement::operator&=(const StorageElement &rhs)
{
	from_u32(to_u32() & rhs.to_u32());
	return *this;
}

StorageElement &StorageElement::operator|=(const StorageElement &rhs)
{
	from_u32(to_u32() | rhs.to_u32());
	return *this;
}

void StorageElement::swap(StorageElement &rhs)
{
	uint32_t tmp = to_u32();
	from_u32(rhs.to_u32());
	rhs.from_u32(tmp);
}

bool StorageElement::get_bit(StorageElement &rhs)
{
	uint32_t tmp = to_u32();
	uint32_t mask = 1 << rhs.to_u32();
	return (tmp & mask) != 0;
}

void StorageElement::set_bit(StorageElement &rhs)
{
	uint32_t tmp = to_u32();
	tmp |= 1 << rhs.to_u32();
	from_u32(tmp);
}

void StorageElement::reset_bit(StorageElement &rhs)
{
	uint32_t tmp = to_u32();
	tmp &= ~(1 << rhs.to_u32()) & 0xFF;
	from_u32(tmp);
}

size_t StorageElement::push(Bus &bus, size_t addr)
{
	bus.write_data(addr-1, ptr[WORD_HI_BYTE_IDX]);
	bus.write_data(addr-2, ptr[WORD_LO_BYTE_IDX]);
	return addr-2;
}

size_t StorageElement::pop(Bus &bus, size_t addr)
{
	ptr[WORD_LO_BYTE_IDX] = bus.read_data(addr);
	ptr[WORD_HI_BYTE_IDX] = bus.read_data(addr+1);
	return addr+2;
}

void StorageElement::shift_right(bool rotate, bool carry_inst, bool carry)
{
	assert(is_8bit());

	uint32_t val = to_u32();
	uint32_t shifted_bit = (val & 0x1) << 7;

	val >>= 1;
	if (rotate && carry_inst)
	{
		val |= shifted_bit;
		flag_carry = (shifted_bit != 0);
	}
	else if (rotate && !carry_inst)
	{
		val |= (carry ? 0x80 : 0x00);
		flag_carry = (shifted_bit != 0);
	}

	from_u32(val & 0xff);
}

void StorageElement::shift_left(bool rotate, bool carry_inst, bool carry)
{
	assert(is_8bit());

	uint32_t val = to_u32();
	uint32_t shifted_bit = (val & 0x80) >> 7;

	val <<= 1;
	if (rotate && carry_inst)
	{
		val |= shifted_bit;
		flag_carry = (shifted_bit != 0);
	}
	else if (rotate && !carry_inst)
	{
		val |= (carry ? 0x1 : 0x0);
		flag_carry = (shifted_bit != 0);
	}
	
	from_u32(val & 0xff);
}

uint32_t StorageElement::to_u32() const
{
	uint32_t v = 0;
	
	switch (count)
	{
	case 1:
		v = static_cast<uint8_t>(*ptr);
		break;
	case 2:
		v = static_cast<uint32_t>(ptr[WORD_LO_BYTE_IDX] & 0xff);
		v |= static_cast<uint32_t>(ptr[WORD_HI_BYTE_IDX] & 0xff) << 8;
		break;
	default:
		assert(false); // Should not get here
	}

	return v;
}

uint32_t StorageElement::to_u32_half() const
{
	uint32_t v = 0;

	switch (count)
	{
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
		assert(false); // Should not get here
	}

	return v;
}

int StorageElement::to_s32() const
{
	int v = 0;

	switch (count)
	{
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

void StorageElement::from_u32(uint32_t v)
{
	switch (count)
	{
	case 1:
		*static_cast<uint8_t *>(ptr) = static_cast<uint8_t>(v);
		break;
	case 2:
		ptr[WORD_LO_BYTE_IDX] = static_cast<uint8_t>(v & 0xff);
		ptr[WORD_HI_BYTE_IDX] = static_cast<uint8_t>((v >> 8) & 0xff);
		break;
	default:
		assert(false); // Should not get here
	}
}

bool StorageElement::significant_bit(bool ishalf) const
{
	uint32_t div = (ishalf ? 2 : 1);
	uint32_t mask = 0x1 << ((8 * count / div) - 1);
	return (to_u32() & mask) != 0;
}

void StorageElement::update_carry(const StorageElement &op1, const StorageElement &op2, bool is_half)
{
	bool res_bit = significant_bit(is_half);
	bool op1_bit = op1.significant_bit(is_half);
	bool op2_bit = op2.significant_bit(is_half);
	bool v = false;

	if (((op1_bit && !op2_bit) || (!op1_bit && op2_bit)) && (!res_bit))
	{
		v = true;
	}

	// If both significant bits are set then we will overflow on addition
	if (op1_bit && op2_bit)
	{
		v = true;
	}

	if (is_half)
	{
		flag_half_carry = v;
	}
	else
	{
		flag_carry = v;
	}
}

void StorageElement::update_borrow(const StorageElement &op1, const StorageElement &op2, bool is_half)
{
	if (is_half)
	{
		if (op1.to_u32_half() < op2.to_u32_half())
		{
			flag_half_carry = true;
		}
		else
		{
			flag_half_carry = false;
		}
	}
	else
	{
		if (op1.to_u32() < op2.to_u32())
		{
			flag_carry = true;
		}
		else
		{
			flag_carry = false;
		}
	}
}

void StorageElement::update_overflow(const StorageElement &op1, const StorageElement &op2)
{
	bool res_bit = significant_bit();
	bool op1_bit = op1.significant_bit();
	bool op2_bit = op2.significant_bit();

	if ((!op1_bit && !op2_bit && res_bit) ||
		(op1_bit && op2_bit && !res_bit))
	{
		flag_overflow = true;
	}
	else
	{
		flag_overflow = false;
	}
}

std::ostream& operator<<(std::ostream& stream, const StorageElement& e)
{
	stream << std::dec << e.to_s32() << " (0x" << std::hex << e.to_u32() << ")" << " [" << e.count << "]" << std::dec;
	return stream;
}
