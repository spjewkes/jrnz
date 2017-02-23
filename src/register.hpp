/**
 * @brief Header defining a register.
 */

#ifndef __REGISTER_HPP__
#define __REGISTER_HPP__

#include "memory.hpp"
#include "storage_element.hpp"
#include "common.hpp"

/**
 * @brief Class describing a 16-bit register.
 */
class Register16
{
public:
	void hi(unsigned char v) { c_reg[WORD_HI_BYTE_IDX] = v; }
	unsigned char hi() const { return c_reg[WORD_HI_BYTE_IDX]; }
	void lo(unsigned char v) { c_reg[WORD_LO_BYTE_IDX] = v; }
	unsigned char lo() const { return c_reg[WORD_LO_BYTE_IDX]; }
	void set(unsigned short v) { reg = v; }
	unsigned short get() const { return reg; }
	void swap() { std::swap(reg, alt_reg); }

	StorageElement element() { return StorageElement(&c_reg[0], 2); }
	StorageElement element_hi() { return StorageElement(&c_reg[WORD_HI_BYTE_IDX], 1); }
	StorageElement element_lo() { return StorageElement(&c_reg[WORD_LO_BYTE_IDX], 1); }

private:
	union
	{
		unsigned short reg = 0;
		unsigned char c_reg[2];
	};
	unsigned short alt_reg = 0;
};

#endif // __REGISTER_HPP__
