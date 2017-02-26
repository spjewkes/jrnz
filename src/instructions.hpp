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
				Conditional _cond, Operand _dst, Operand _src)
		: inst(_inst), name(_name), size(_size), cycles(_cycles), cond(_cond), dst(_dst), src(_src) {}
	
	InstType inst;
	std::string name;
	unsigned int size;
	unsigned int cycles;
	Conditional cond;
	Operand dst;
	Operand src;

	bool execute(Z80 &state);

private:
	bool do_nop(Z80 &state);
	bool do_ld(Z80 &state);
	bool do_xor(Z80 &state);
	bool do_jp(Z80 &state);
	bool do_di(Z80 &state);
	bool do_out(Z80 &state);
	bool do_sub(Z80 &state);
	bool do_cp(Z80 &state);
} Instruction;

/**
 * @brief Define protoypes for instruction callbacks
 */
bool inst_nop(Z80 &state, Operand dst, Operand src);
bool inst_ld(Z80 &state, Operand dst, Operand src);
bool inst_xor(Z80 &state, Operand dst, Operand src);
bool inst_jp(Z80 &state, Operand dst, Operand src);
bool inst_di(Z80 &state, Operand dst, Operand src);
bool inst_out(Z80 &state, Operand dst, Operand src);
bool inst_sub(Z80 &state, Operand dst, Operand src);
bool inst_cp(Z80 &state, Operand dst, Operand src);

#endif // __INSTRUCTIONS_HPP__
