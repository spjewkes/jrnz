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

StorageElement &StorageElement::operator=(const StorageElement &rhs)
{
	assert(count == rhs.count);
	if ((this != &rhs) && (!readonly))
	{
		std::memcpy(ptr, rhs.ptr, count);
	}
	return *this;
}

StorageElement StorageElement::operator+(const StorageElement &rhs)
{
	//! TODO fix issue if count is accidentally missed on constructor
	return StorageElement(to_s32() + rhs.to_s32(), count);
}

StorageElement StorageElement::operator-(const StorageElement &rhs)
{
	//! TODO fix issue if count is accidentally missed on constructor
	return StorageElement(to_s32() - rhs.to_s32(), count);
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

void StorageElement::do_copy(const StorageElement &rhs)
{
	*this = rhs;
}

void StorageElement::do_xor(const StorageElement &rhs, Z80 &state)
{
	unsigned int a = to_u32();
	unsigned int b = rhs.to_u32();
	a ^= b;
	from_u32(a);

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, is_zero());
	state.af.flag(RegisterAF::Flags::Sign, is_neg());
}

void StorageElement::do_and(const StorageElement &rhs, Z80 &state)
{
	unsigned int a = to_u32();
	unsigned int b = rhs.to_u32();
	a &= b;
	from_u32(a);

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.flag(RegisterAF::Flags::ParityOverflow, is_even_parity());
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.flag(RegisterAF::Flags::Zero, is_zero());
	state.af.flag(RegisterAF::Flags::Sign, is_neg());
}

void StorageElement::do_subtract(const StorageElement &rhs, Z80 &state, bool update_state, bool store, bool use_carry)
{
	StorageElement carry(state.af.flag(RegisterAF::Flags::Carry) ? 1 : 0);
	StorageElement result = *this - rhs - carry;

	if (update_state)
	{
		state.af.set_borrow(to_u32(), rhs.to_u32(), result.to_u32(), count);
		state.af.flag(RegisterAF::Flags::AddSubtract, true);
		state.af.set_overflow(to_u32(), rhs.to_u32(), result.to_u32(), count);
		state.af.set_borrow(to_u32(), rhs.to_u32(), result.to_u32(), count, true);
		state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
		state.af.flag(RegisterAF::Flags::Sign, result.is_neg());
	}

	if (store)
	{
		*this = result;
	}
}

void StorageElement::do_jr(const StorageElement &rhs, Z80 &state, Conditional cond)
{
	StorageElement result = *this + rhs;

	if (is_cond_set(cond, state))
	{
		*this = result;
	}
}

bool StorageElement::is_cond_set(Conditional cond, Z80 &state)
{
	switch(cond)
	{
	case Conditional::Z:  return state.af.flag(RegisterAF::Flags::Zero);
	case Conditional::NZ: return !state.af.flag(RegisterAF::Flags::Zero);
	case Conditional::C:  return state.af.flag(RegisterAF::Flags::Carry);
	case Conditional::NC: return !state.af.flag(RegisterAF::Flags::Carry);
	default:
		return false;
	}
}

void StorageElement::do_addition(const StorageElement &rhs, Z80 &state, bool update_state)
{
	StorageElement result = *this + rhs;

	state.af.set_borrow(to_u32(), rhs.to_u32(), result.to_u32(), count);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.set_overflow(to_u32(), rhs.to_u32(), result.to_u32(), count);
	state.af.set_borrow(to_u32(), rhs.to_u32(), result.to_u32(), count, true);
	state.af.flag(RegisterAF::Flags::Zero, result.is_zero());
	state.af.flag(RegisterAF::Flags::Sign, result.is_neg());

	if (update_state)
	{
		*this = result;
	}
}
