/**
 * @brief Header file handling instruction code.
 */

#ifndef __INSTRUCTIONS_HPP__
#define __INSTRUCTIONS_HPP__

#include <string>
#include "common.hpp"

/**
 * @brief Forward declarations
 */
class Z80;

/**
 * @brief Defines an instruction entry
 */
class Instruction
{
public:
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				Operand _dst = Operand::UNUSED, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				unsigned int _cycles_not_cond, Operand _dst, Operand _src)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cycles_not_cond(_cycles_not_cond), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				unsigned int _cycles_not_cond, Conditional _cond, Operand _dst, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cycles_not_cond(_cycles_not_cond), cond(_cond), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				Conditional _cond, Operand _dst, Operand _src = Operand::UNUSED)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(_cond), dst(_dst), src(_src) {}

	InstType inst;
	std::string name;
	unsigned int size;
	unsigned int cycles = { 0 };
	unsigned int cycles_not_cond = { 0 };
	Conditional cond = { Conditional::UNUSED };
	Operand dst;
	Operand src;

	void execute(Z80 &state);

private:
	void do_nop(Z80 &state);
	void do_ld(Z80 &state);
	void do_lddr(Z80 &state);
	void do_ldir(Z80 &state);
	void do_xor(Z80 &state);
	void do_and(Z80 &state);
	void do_or(Z80 &state);
	void do_jp(Z80 &state);
	void do_di(Z80 &state);
	void do_ei(Z80 &state);
	void do_im(Z80 &state);
	void do_out(Z80 &state);
	void do_ex(Z80 &state);
	void do_dec(Z80 &state);
	void do_cp(Z80 &state);
	void do_jr(Z80 &state);
	void do_djnz(Z80 &state);
	void do_call(Z80 &state);
	void do_ret(Z80 &state);
	void do_bit(Z80 &state);
	void do_set(Z80 &state);
	void do_res(Z80 &state);
	void do_sub(Z80 &state);
	void do_sbc(Z80 &state);
	void do_add(Z80 &state);
	void do_inc(Z80 &state);
	void do_push(Z80 &state);
	void do_pop(Z80 &state);
	void do_rrca(Z80 &state);
	void do_scf(Z80 &state);
	void do_ccf(Z80 &state);
	void do_rst(Z80 &state);

	void impl_ld_block(Z80 &state, bool inc);
	void impl_set_bit(Z80 &state, bool set);
	void impl_add(Z80 &state, bool store, bool is_inc);
	void impl_sub(Z80 &state, bool store, bool use_carry, bool is_dec);

	bool is_cond_set(Conditional cond, Z80 &state);
};

#endif // __INSTRUCTIONS_HPP__
