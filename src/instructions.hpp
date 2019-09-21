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

	size_t execute(Z80 &state);

private:
	// All instructions have the same signature regardless of whether they make use of them or not.
    size_t do_nop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ld(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_lddr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ldir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_xor(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_and(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_or(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_jp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_di(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ei(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_im(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_in(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_out(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ex(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_dec(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cp(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_jr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_djnz(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_call(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ret(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_retn(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_reti(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_set(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_res(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_sbc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_adc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_inc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_push(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_pop(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rlc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rrc(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_sla(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_sll(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_sra(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_srl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rlca(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rla(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_scf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_ccf(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cpl(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cpi(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cpir(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cpd(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_cpdr(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_rst(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);
	size_t do_halt(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem);

	size_t impl_ld_block(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool inc);
	size_t impl_set_bit(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool set);
	size_t impl_add(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry, bool is_inc);
	size_t impl_sub(Z80 &state, StorageElement &dst_elem, StorageElement &src_elem, bool store, bool use_carry, bool is_dec);
	size_t impl_shift_left(Z80 &state, StorageElement &elem, bool set_state, bool rotate, bool carry_inst);
	size_t impl_shift_right(Z80 &state, StorageElement &elem, bool set_state, bool rotate, bool carry_inst);
	size_t impl_cp_inc_dec(Z80 &state, bool do_inc, bool loop);
	size_t impl_ret(Z80 &state, StorageElement &pc);

	bool is_cond_set(Conditional cond, Z80 &state);
};

#endif // __INSTRUCTIONS_HPP__
