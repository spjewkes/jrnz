/**
 * @brief Implementation of storage element class.
 */

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

StorageElement StorageElement::create_element(Z80 &state, Operand operand, unsigned short old_pc, bool &handled)
{
	handled = true;

	switch(operand)
	{
	case BC:     return state.bc.element();
	case DE:     return state.de.element();
	case HL:     return state.hl.element();
	case SP:     return state.sp.element();
	case A:      return state.af.element_hi();
	case B:      return state.bc.element_hi();
	case C:      return state.bc.element_lo();
	case D:      return state.de.element_hi();
	case E:      return state.de.element_lo();
	case H:      return state.hl.element_hi();
	case L:      return state.hl.element_lo();
	case N:      return StorageElement(state.mem.read(old_pc+1));
	case NN:     return StorageElement(state.mem.read(old_pc+1), state.mem.read(old_pc+2));
	case PC:     return state.pc.element();
	case PORT:   return state.ports.element(state.mem.read(old_pc+1));
	case UNUSED:
	default:
		handled = false;
		return StorageElement(nullptr, 0);
	}
}

void StorageElement::do_load(const StorageElement &rhs)
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
	assert(count == 1); // assume 1 byte size for now

	*ptr ^= *rhs.ptr;

	state.af.flag(RegisterAF::Flags::Carry, false);
	state.af.flag(RegisterAF::Flags::AddSubtract, false);
	state.af.set_parity(*ptr);
	state.af.flag(RegisterAF::Flags::HalfCarry, false);
	state.af.set_zero(*ptr);
	state.af.set_negative(*ptr);
}

void StorageElement::do_jmp(const StorageElement &rhs)
{
	assert(count == rhs.count);

	if ((this != &rhs) && (!readonly))
	{
		std::memcpy(ptr, rhs.ptr, count);
	}
}

void StorageElement::do_out(const StorageElement &rhs)
{
	assert(count == rhs.count);
	assert(1 == count);

	if ((this != &rhs) && (!readonly))
	{
		std::memcpy(ptr, rhs.ptr, count);
	}
}


