/**
 * @brief Implementation of storage element class.
 */

#include "common.hpp"
#include "storage_element.hpp"
#include "z80.hpp"

StorageElement::StorageElement(unsigned char *_ptr, size_t _count, bool _readonly) : ptr(_ptr), count(_count), readonly(_readonly)
{
	if (readonly)
	{
		read_only.resize(count);
		std::memcpy(&read_only[0], _ptr, _count);
	}
}

StorageElement::StorageElement(unsigned char v) : ptr(nullptr), count(1), readonly(true)
{
	read_only.push_back(v);
	ptr = &read_only[0];
}

StorageElement::StorageElement(unsigned char lo, unsigned char hi) : ptr(nullptr), count(2), readonly(true)
{
	read_only.resize(count);
	read_only[WORD_LO_BYTE_IDX] = lo;
	read_only[WORD_HI_BYTE_IDX] = hi;
	ptr = &read_only[0];
}

StorageElement::StorageElement(unsigned int v, size_t _count) : count(_count), readonly(true)
{
	assert((count == 1) || (count == 2));
	read_only.resize(count);
	ptr = &read_only[0];
	
	switch (count)
	{
	case 1:
		read_only[WORD_LO_BYTE_IDX] = static_cast<unsigned char>(v);
		break;
	case 2:
		ptr[WORD_LO_BYTE_IDX] = static_cast<unsigned char>(v & 0xff);
		ptr[WORD_HI_BYTE_IDX] = static_cast<unsigned char>((v >> 8) & 0xff);
		break;
	default:
		assert(false); // Should not get here
	}
}

StorageElement StorageElement::create_element(Z80 &state, Operand operand, bool &handled)
{
	handled = true;

	switch(operand)
	{
	case Operand::BC:     return state.bc.element();
	case Operand::DE:     return state.de.element();
	case Operand::HL:     return state.hl.element();
	case Operand::SP:     return state.sp.element();
	case Operand::A:      return state.af.element_hi();
	case Operand::B:      return state.bc.element_hi();
	case Operand::C:      return state.bc.element_lo();
	case Operand::D:      return state.de.element_hi();
	case Operand::E:      return state.de.element_lo();
	case Operand::H:      return state.hl.element_hi();
	case Operand::L:      return state.hl.element_lo();
	case Operand::N:      return StorageElement(state.mem.read(state.curr_operand_pc));
	case Operand::NN:     return StorageElement(state.mem.read(state.curr_operand_pc), state.mem.read(state.curr_operand_pc+1));
	case Operand::PC:     return state.pc.element();
	case Operand::PORT:   return state.ports.element(state.mem.read(state.curr_operand_pc));
	case Operand::I:      return state.ir.element_hi();
	case Operand::indHL:  return StorageElement(state.mem.read(state.hl.get()));
    case Operand::ONE:    return StorageElement(0x1);
	case Operand::UNUSED:
	default:
		handled = false;
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
	StorageElement result = StorageElement(to_s32() + rhs.to_s32(), count);
	result.update_borrow(*this, rhs);
	result.update_borrow(*this, rhs, true /* is_half */);
	result.update_overflow(*this, rhs);
	return StorageElement(to_s32() - rhs.to_s32(), count);
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

bool StorageElement::is_zero() const
{
	return (to_u32() == 0);
}

bool StorageElement::is_neg() const
{
	return (to_s32() < 0);
}

bool StorageElement::is_even_parity() const
{
	return (std::bitset<32>(to_u32()).count() % 2);
}

bool StorageElement::is_carry() const
{
	return flag_carry;
}

bool StorageElement::is_half() const
{
	return flag_half_carry;
}

bool StorageElement::is_overflow() const
{
	return flag_overflow;
}

unsigned int StorageElement::to_u32() const
{
	unsigned int v = 0;
	
	switch (count)
	{
	case 1:
		v = static_cast<unsigned char>(*ptr);
		break;
	case 2:
		v = static_cast<unsigned int>(ptr[WORD_LO_BYTE_IDX] & 0xff);
		v |= static_cast<unsigned int>(ptr[WORD_HI_BYTE_IDX] & 0xff) << 8;
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

void StorageElement::from_u32(unsigned int v)
{
	switch (count)
	{
	case 1:
		*static_cast<unsigned char *>(ptr) = static_cast<unsigned char>(v);
		break;
	case 2:
		ptr[WORD_LO_BYTE_IDX] = static_cast<unsigned char>(v & 0xff);
		ptr[WORD_HI_BYTE_IDX] = static_cast<unsigned char>((v >> 8) & 0xff);
		break;
	default:
		assert(false); // Should not get here
	}
}

bool StorageElement::significant_bit(bool ishalf) const
{
	unsigned int div = (ishalf ? 2 : 1);
	unsigned int mask = 0x1 << ((8 * count / div) - 1);
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
	bool res_bit = significant_bit(is_half);
	bool op1_bit = op1.significant_bit(is_half);
	bool op2_bit = op2.significant_bit(is_half);
	bool v = false;

	if (!op1_bit && !op2_bit && res_bit)
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
