/**
 * @brief Defines the storage element class which is used to manage operations generically.
 */

#ifndef __STORAGE_ELEMENT_HPP__
#define __STORAGE_ELEMENT_HPP__

#include <vector>
#include <bitset>
#include <cstdint>
#include "common.hpp"

class Z80;
class Bus;

/**
 * @brief Defines a standard storage element type to allow operations between registers/memory.
 */
class StorageElement
{
public:
	explicit StorageElement(uint8_t *_ptr, size_t _count, bool _readonly=false);
	explicit StorageElement(uint8_t v);
	explicit StorageElement(uint8_t lo, uint8_t hi);

	static StorageElement create_element(Z80 &state, Operand operand);

	StorageElement &operator=(const StorageElement &rhs);
	StorageElement &operator=(const uint8_t rhs);
	StorageElement operator+(const StorageElement &rhs);
	StorageElement operator-(const StorageElement &rhs);
	StorageElement &operator^=(const StorageElement &rhs);
	StorageElement &operator&=(const StorageElement &rhs);
	StorageElement &operator|=(const StorageElement &rhs);
	void swap(StorageElement &rhs);
	bool get_bit(StorageElement &rhs);
	void set_bit(StorageElement &rhs);
	void reset_bit(StorageElement &rhs);
	size_t push(Bus &bus, size_t addr);
	size_t pop(Bus &bus, size_t addr);
	void shift_right(bool rotate, bool carry_inst, bool carry);
	void shift_left(bool rotate, bool carry_inst, bool carry);

	/**
	 * Query functions.
	 */
	bool is_zero() const { return (to_u32() == 0); }
	bool is_neg() const { return (to_s32() < 0); }
	bool is_even_parity() const { return (std::bitset<32>(to_u32()).count() % 2) == 0; }
	bool is_carry() const { return flag_carry; }
	bool is_half() const { return flag_half_carry; }
	bool is_overflow() const { return flag_overflow; }
	bool is_8bit() const { return count == 1; }
	bool is_16bit() const { return count == 2; }
	void get_value(uint32_t &val) const { val = to_u32(); }

	friend std::ostream& operator<<(std::ostream& stream, const StorageElement& e);

private:
	explicit StorageElement(uint32_t v, size_t _count);

	/**
	 * Conversion to/from storage element.
	 */
	uint32_t to_u32() const;
	uint32_t to_u32_half() const;
	int to_s32() const;
	void from_u32(uint32_t v);

	/**
	 * Updates carry/overflow flags based on particular operations.
p	 */
	bool significant_bit(bool ishalf=false) const;
	void update_carry(const StorageElement &op1, const StorageElement &op2, bool is_half=false);
	void update_borrow(const StorageElement &op1, const StorageElement &op2, bool is_half=false);
	void update_overflow(const StorageElement &op1, const StorageElement &op2);

	uint8_t *ptr;
	size_t count;

	bool flag_carry = false;
	bool flag_half_carry = false;
	bool flag_overflow = false;

	std::vector<uint8_t> read_only; // This should only be used if the readonly flag is set
	bool readonly; //! TODO - this should be a specialized version of StorageElement
};

std::ostream& operator<<(std::ostream& stream, const StorageElement& e);

#endif // __STORAGE_ELEMENT_HPP__
