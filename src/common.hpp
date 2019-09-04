/**
 * @brief Header containing common definitions.
 */

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

#include <cassert>
#include <iostream>

/**
 * @brief Define the high and low bytes in a short word
 */
#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN ||	\
	defined(__BIG_ENDIAN__) || \
	defined(__ARMEB__) || \
	defined(__THUMBEB__) || \
	defined(__AARCH64EB__) || \
	defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)

// It's a big-endian target architecture

/* These defines are specifically for access a byte stream mapping onto a word */
#define WORD_HI_BYTE_IDX (0)
#define WORD_LO_BYTE_IDX (1)

#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
	defined(__LITTLE_ENDIAN__) || \
	defined(__ARMEL__) || \
	defined(__THUMBEL__) || \
	defined(__AARCH64EL__) || \
	defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)

// It's a little-endian target architecture

/* These defines are specifically for access a byte stream mapping onto a word */
#define WORD_HI_BYTE_IDX (1)
#define WORD_LO_BYTE_IDX (0)

#else
#error "Unknown architecture. Cannot detect endianess"
#endif

/**
 * Mark a variable as unused to silence compiler errors.
 */
#define UNUSED(expr) do { (void)(expr); } while (0)

/**
 * @brief Defines instruction type.
 */
enum class InstType
{
	INV,
	NOP,
	LD,
	AND,
	XOR,
	OR,
	JP,
	DI,
	EI,
	OUT,
	DEC,
	CP,
	JR,
	DJNZ,
	SUB,
	SBC,
	ADD,
	INC,
	EX,
	LDDR,
	LDIR,
	IM,
	BIT,
	SET,
	RES,
	CALL,
	RET,
	PUSH,
	POP,
	RRA,
	RRCA,
	SCF,
	CCF,
	RST,
};

/**
 * @brief Defined the conditionals passed into an instruction handling function
 */
enum class Conditional
{
	ALWAYS,
	Z,
	NZ,
	C,
	NC,
	M,
	P,
	PE,
	PO,
	UNUSED
};

/**
 * @brief Defines the operands that can be passed into an instruction handling function
 */
enum class Operand
{
	AF,
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
	PC,
	PORT,
	I,
	R,
	IX,
	IY,
	indBC,
	indDE,
	indHL,
	indN,
	indNN,
	indIXN,
	indIYN,
	indSP,
	ZERO,
	ONE,
	TWO,
	THREE,
	FOUR,
	FIVE,
	SIX,
	SEVEN,
	HEX_0000,
	HEX_0008,
	HEX_0010,
	HEX_0018,
	HEX_0020,
	HEX_0028,
	HEX_0030,
	HEX_0038,
	IM,
	UNUSED
};

#endif // __COMMON_HPP__
