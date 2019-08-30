/**
 * @brief Header file handling instruction code.
 */

#ifndef __INSTRUCTIONS_HPP__
#define __INSTRUCTIONS_HPP__

#include <string>
#include <cstdint>
#include "common.hpp"

/**
 * @brief Forward declarations
 */
class Z80;
class StorageElement;

/**
 * @brief Defines an instruction entry
 */
class Instruction
{
public:
	Instruction(InstType _inst, const char *_name, size_t _size, size_t _cycles,
				Operand _dst = Operand::UNUSED, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, size_t _size, size_t _cycles,
				size_t _cycles_not_cond, Operand _dst, Operand _src)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cycles_not_cond(_cycles_not_cond), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, size_t _size, size_t _cycles,
				size_t _cycles_not_cond, Conditional _cond, Operand _dst, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cycles_not_cond(_cycles_not_cond), cond(_cond), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, size_t _size, size_t _cycles,
				Conditional _cond, Operand _dst, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(_cond), dst(_dst), src(_src) {}

	InstType inst;
	std::string name;
	size_t size;
	size_t cycles = { 0 };
	size_t cycles_not_cond = { 0 };
	Conditional cond = { Conditional::UNUSED };
	Operand dst;
	Operand src;

	void execute(Z80 &state);

private:
	// All instructions have the same signature regardless of whether they make use of them or not.
	void do_nop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ld(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_lddr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ldir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_xor(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_and(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_or(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_jp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_di(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ei(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_im(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_out(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ex(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_dec(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_cp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_jr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_djnz(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_call(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ret(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_set(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_res(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_sbc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_inc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_push(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_pop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_rra(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_rrca(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_scf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_ccf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	void do_rst(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);

	void impl_ld_block(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool inc);
	void impl_set_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool set);
	void impl_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool is_inc);
	void impl_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry, bool is_dec);

	bool is_cond_set(Conditional cond, Z80 &state);
};

#endif // __INSTRUCTIONS_HPP__
