/**
 * @brief Defines the storage element class which is used to manage operations generically.
 */

#ifndef __STORAGE_ELEMENT_HPP__
#define __STORAGE_ELEMENT_HPP__

#include <vector>
#include "common.hpp"

class Z80;

/**
 * @brief Defines a standard storage element type to allow operations between registers/memory.
 */
class StorageElement
{
public:
	explicit StorageElement(unsigned char *_ptr, size_t _count, bool _readonly=false);
	explicit StorageElement(unsigned char v);
	explicit StorageElement(unsigned char lo, unsigned char hi);

	static StorageElement create_element(Z80 &state, Operand operand, bool &handled);

	StorageElement &operator=(const StorageElement &rhs);
	StorageElement operator+(const StorageElement &rhs);
	StorageElement operator-(const StorageElement &rhs);
	StorageElement &operator^=(const StorageElement &rhs);
	StorageElement &operator&=(const StorageElement &rhs);
	void swap(StorageElement &rhs);

	/**
	 * Query functions.
	 */
	bool is_zero() const { return (to_u32() == 0); }
	bool is_neg() const { return (to_s32() < 0); }
	bool is_even_parity() const { return (std::bitset<32>(to_u32()).count() % 2); }
	bool is_carry() const { return flag_carry; }
	bool is_half() const { return flag_half_carry; }
	bool is_overflow() const { return flag_overflow; }
	bool is_8bit() const { return count == 1; }
	bool is_16bit() const { return count == 2; }

	friend std::ostream& operator<<(std::ostream& stream, const StorageElement& e);

private:
	explicit StorageElement(unsigned int v, size_t _count);

	/**
	 * Conversion to/from storage element.
	 */
	unsigned int to_u32() const;
	int to_s32() const;
	void from_u32(unsigned int v);

	/**
	 * Updates carry/overflow flags based on particular operations.
	 */
	bool significant_bit(bool ishalf=false) const;
	void update_carry(const StorageElement &op1, const StorageElement &op2, bool is_half=false);
	void update_borrow(const StorageElement &op1, const StorageElement &op2, bool is_half=false);
	void update_overflow(const StorageElement &op1, const StorageElement &op2);

	unsigned char *ptr;
	size_t count;

	bool flag_carry = false;
	bool flag_half_carry = false;
	bool flag_overflow = false;

	std::vector<unsigned char> read_only; // This should only be used if the readonly flag is set
	bool readonly; //! TODO - this should be a specialized version of StorageElement
};

std::ostream& operator<<(std::ostream& stream, const StorageElement& e);

#endif // __STORAGE_ELEMENT_HPP__
