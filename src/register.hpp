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
	void hi(unsigned char v) { reg = ((v << 8) & 0xff00) | (reg & 0xff); }
	unsigned char hi() const { return static_cast<unsigned char>((reg>>8) & 0xff); }
	void lo(unsigned char v) { reg = (reg & 0xff00) | (v & 0xff); }
	unsigned char lo() const { return static_cast<unsigned char>(reg & 0xff); }
	void set(unsigned short v) { reg = v; }
	unsigned short get() const { return reg; }
	void swap() { std::swap(reg, alt_reg); }

	StorageElement element() { return StorageElement(&c_reg[0], 2); }
	StorageElement element_hi() { return StorageElement(&c_reg[HI_BYTE], 1); }
	StorageElement element_lo() { return StorageElement(&c_reg[LO_BYTE], 1); }

private:
	union
	{
		unsigned short reg = 0;
		unsigned char c_reg[2];
	};
	unsigned short alt_reg = 0;
};

#endif // __REGISTER_HPP__
