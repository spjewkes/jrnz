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

	/**
	 * Query functions functions.
	 */
	bool is_zero() const;
	bool is_neg() const;
	bool is_even_parity() const;
	bool is_cond_set(Conditional cond, Z80 &state);

	/**
	 * Conversion to/from storage element.
	 * TODO: these should be hidden eventually
	 */
	unsigned int to_u32() const;
	int to_s32() const;
	void from_u32(unsigned int v);
	size_t size() const { return count; }

private:
	explicit StorageElement(unsigned int v, size_t _count);

	unsigned char *ptr;
	std::vector<unsigned char> read_only; // This should only be used if the readonly flag is set
	size_t count;
	bool readonly; //! TODO - this should be a specialized version of StorageElement
};

#endif // __STORAGE_ELEMENT_HPP__
