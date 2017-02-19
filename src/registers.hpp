/**
 * @brief Header managing registers and thier state.
 */

#ifndef __REGISTERS_HPP__
#define __REGISTERS_HPP__

#include "memory.hpp"

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN ||	\
	defined(__BIG_ENDIAN__) || \
	defined(__ARMEB__) || \
	defined(__THUMBEB__) || \
	defined(__AARCH64EB__) || \
	defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)
// It's a big-endian target architecture
#define HI_REG8 (1)
#define LO_REG8 (0)
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
	defined(__LITTLE_ENDIAN__) || \
	defined(__ARMEL__) || \
	defined(__THUMBEL__) || \
	defined(__AARCH64EL__) || \
	defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
// It's a little-endian target architecture
#define HI_REG8 (0)
#define LO_REG8 (1)
#else
#error "I don't know what architecture this is!"
#endif

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
	StorageElement element_hi() { return StorageElement(&c_reg[HI_REG8], 1); }
	StorageElement element_lo() { return StorageElement(&c_reg[LO_REG8], 1); }

private:
	union
	{
		unsigned short reg = 0;
		unsigned char c_reg[2];
	};
	unsigned short alt_reg = 0;
};

#endif // __REGISTERS_HPP__
