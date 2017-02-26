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

void StorageElement::do_copy(const StorageElement &rhs)
{
	assert(count == rhs.count);
	if ((this != &rhs) && (!readonly))
	{
		std::memcpy(ptr, rhs.ptr, count);
	}
}

void StorageElement::do_xor(const StorageElement &rhs, Z80 &state)
{
	assert(count == rhs.count);
	assert(count == 1); // Only handles 1 byte case

	*ptr ^= *rhs.ptr;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.set_parity(*ptr);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.set_zero(*ptr);
	state.af.set_negative(*ptr);
}

void StorageElement::do_subtract(const StorageElement &rhs, Z80 &state, bool update_state, bool store)
{
	unsigned int a = convert_to_u32(ptr, count);
	unsigned int b = convert_to_u32(rhs.ptr, rhs.count);

	unsigned int result = a - b;

	if (store)
	{
		convert_to_array(ptr, count, result);
	}

	if (update_state)
	{
		state.af.set_borrow(a, b, result, count);
		state.af.flag(RegisterAF::Flags::AddSubtract, true);
		state.af.set_overflow(a, b, result, count);
		state.af.set_borrow(a, b, result, count, true);
		state.af.set_zero(result);
		state.af.set_negative(result);
	}
}

void StorageElement::do_jr(const StorageElement &rhs, Z80 &state, Conditional cond)
{
	unsigned int a = convert_to_u32(ptr, count);
	int b = convert_to_s32(rhs.ptr, rhs.count);

	unsigned int result = a + b;

	if (check_condition(cond, state))
	{
		convert_to_array(ptr, count, result);
	}
}

bool StorageElement::check_condition(Conditional cond, Z80 &state)
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
