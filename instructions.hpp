/**
 * @brief Header file handling instruction code.
 */

#ifndef __INSTRUCTIONS_HPP__
#define __INSTRUCTIONS_HPP__

#include <string>

class Z80;

enum Operand
{
	BC,
	DE,
	HL,
	SP,
	A,
	B,
	C,
	D,
	E,
	H,
	L,
	N,
	NN,
	UNUSED
};

typedef bool(*inst_fn)(Z80&,unsigned short, Operand, Operand);

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

bool inst_ld(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_xor(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_jp_nn(Z80 &state, unsigned short old_pc, Operand dst, Operand src);
bool inst_di(Z80 &state, unsigned short old_pc, Operand dst, Operand src);

#endif // __INSTRUCTIONS_HPP__
