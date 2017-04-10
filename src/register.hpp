/**
 * @brief Header defining a register.
 */

#ifndef __REGISTER_HPP__
#define __REGISTER_HPP__

#include <bitset>
#include "memory.hpp"
#include "storage_element.hpp"
#include "common.hpp"

/**
 * @brief Class describing a 16-bit register.
 */
class Register16
{
public:
	void reset() { reg = 0x00; alt_reg = 0x00; }
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

	friend std::ostream& operator<<(std::ostream& stream, const Register16& e);

private:
	union
	{
		unsigned short reg = 0;
		unsigned char c_reg[2];
	};
	unsigned short alt_reg = 0;
};

std::ostream& operator<<(std::ostream& stream, const Register16& e);

/**
 * @brief Specialization of Register16 for A and F registers.
 */
class RegisterAF : public Register16
{
public:
	enum class Flags
	{
		Carry = 0,
		AddSubtract = 1,
		ParityOverflow = 2,
		HalfCarry = 4,
		Zero = 6,
		Sign = 7
	};

	void accum(unsigned char v) { hi(v); }
	unsigned char accum() const { return hi(); }
	void flags(unsigned char v) { lo(v); }
	unsigned char flags() const { return lo(); }

	void flag(Flags f, bool v)
		{
			unsigned char bit = 0x1 << static_cast<unsigned int>(f);
			flags((flags() & ~bit) | (v?bit:0));
		}
	bool flag(Flags f)
		{
			unsigned char bit = 0x1 << static_cast<unsigned int>(f);
			return (flags() & bit) != 0;
		}
	void inv_flag(Flags f)
		{
			if (flag(f))
				flag(f, false);
			else
				flag(f, true);
		}

private:
	bool sig_bit(unsigned int v, size_t count, bool ishalf=false)
		{
			unsigned int div = (ishalf ? 2 : 1);
			unsigned int mask = 0x1 << ((8 * count / div) - 1);
			return (v & mask) != 0;
		}
};

#endif // __REGISTER_HPP__
