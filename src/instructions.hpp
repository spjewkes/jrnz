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
typedef bool(*inst_fn)(Z80&,unsigned short, Operand, Operand);

/**
 * @brief Defines an instruction entry
 */
typedef struct Instruction
{
public:
	std::string name;
	unsigned int size;
	unsigned int cycles;
	inst_fn func;
	Operand dst;
	Operand src;
} Instruction;

/**
 * @brief Define protoypes for instruction callbacks
 */
bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_xor(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_jp_nn(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_di(Z80 &state, unsigned short old_pc, Operand dst, Operand src);

#endif // __INSTRUCTIONS_HPP__
