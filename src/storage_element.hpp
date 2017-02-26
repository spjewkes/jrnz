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
	StorageElement(unsigned char *_ptr, size_t _count, bool _readonly=false);
	StorageElement(unsigned char v);
	StorageElement(unsigned char lo, unsigned char hi);

	static StorageElement create_element(Z80 &state, Operand operand, bool &handled);

	void do_copy(const StorageElement &rhs);
	void do_xor(const StorageElement &rhs, Z80 &state);
	void do_subtract(const StorageElement &rhs, Z80 &state, bool update_state=true, bool store=true);
	void do_jr(const StorageElement &rhs, Z80 &state, Conditional cond);

private:
	bool check_condition(Conditional cond, Z80 &state);

	unsigned char *ptr;
	std::vector<unsigned char> read_only;
	size_t count;
	bool readonly;
};

#endif // __STORAGE_ELEMENT_HPP__
