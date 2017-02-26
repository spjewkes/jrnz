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
 * @brief Defines the callback function used to handle each instruction
 */
typedef bool(*inst_fn)(Z80&, Operand, Operand);

/**
 * @brief Defines an instruction entry
 */
typedef struct Instruction
{
public:
	Instruction(const char *_name, unsigned int _size, unsigned int _cycles, inst_fn _func,
				Operand _dst, Operand _src)
		: name(_name), size(_size), cycles(_cycles), func(_func), cond(Conditional::UNUSED), dst(_dst), src(_src) {}
	Instruction(const char *_name, unsigned int _size, unsigned int _cycles, inst_fn _func,
				Conditional _cond, Operand _dst, Operand _src)
		: name(_name), size(_size), cycles(_cycles), func(_func), cond(_cond), dst(_dst), src(_src) {}
	
	std::string name;
	unsigned int size;
	unsigned int cycles;
	inst_fn func;
	Conditional cond;
	Operand dst;
	Operand src;
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
