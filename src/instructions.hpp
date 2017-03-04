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
typedef struct Instruction
{
public:
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				Operand _dst, Operand _src)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(InstType _inst, const char *_name, unsigned int _size, unsigned int _cycles,
				unsigned int _cycles_not_cond, Conditional _cond, Operand _dst, Operand _src)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cycles_not_cond(_cycles_not_cond), cond(_cond), dst(_dst), src(_src) {}
	
	InstType inst;
	std::string name;
	unsigned int size;
	unsigned int cycles;
	unsigned int cycles_not_cond;
	Conditional cond;
	Operand dst;
	Operand src;

	bool execute(Z80 &state);

private:
	bool do_nop(Z80 &state);
	bool do_ld(Z80 &state);
	bool do_xor(Z80 &state);
	bool do_and(Z80 &state);
	bool do_jp(Z80 &state);
	bool do_di(Z80 &state);
	bool do_out(Z80 &state);
	bool do_dec(Z80 &state);
	bool do_cp(Z80 &state);
	bool do_jr(Z80 &state);
	bool do_sbc(Z80 &state);
	bool do_add(Z80 &state);
	bool do_inc(Z80 &state);
} Instruction;

#endif // __INSTRUCTIONS_HPP__
