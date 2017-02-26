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
 * @brief Defines instruction type.
 */
enum class InstType
{
	NOP,
	LD,
	XOR,
	JP,
	DI,
	OUT,
	SUB,
	CP,
	JR,
};

/**
 * @brief Defined the conditionals passed into an instruction handling function
 */
enum class Conditional
{
	Z,
	NZ,
	C,
	NC,
	UNUSED
};

/**
 * @brief Defines the operands that can be passed into an instruction handling function
 */
enum class Operand
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
	PC,
	PORT,
	I,
	indHL,
	UNUSED
};

/**
 * @brief Convert byte array to an unsigned int.
 */
static unsigned int convert_to_u32(const unsigned char *array, size_t count)
{
	unsigned int v = 0;
	
	switch (count)
	{
	case 1:
		v = static_cast<unsigned char>(*array);
		break;
	case 2:
		v = static_cast<unsigned int>(array[WORD_LO_BYTE_IDX] & 0xff);
		v |= static_cast<unsigned int>(array[WORD_HI_BYTE_IDX] & 0xff) << 8;
		break;
	default:
		assert(false); // Should not get here
	}

	return v;
}

/**
 * @brief Convert byte array to a signed int.
 */
static int convert_to_s32(const unsigned char *array, size_t count)
{
	int v = 0;

	switch (count)
	{
	case 1:
		v = static_cast<char>(*array);
		break;
	case 2:
		short w = static_cast<short>(array[WORD_LO_BYTE_IDX] & 0xff);
		w |= static_cast<short>(array[WORD_HI_BYTE_IDX] & 0xff) << 8;
		v = w;
	}

	return v;
}

/**
 * @brief Convert unsigned int to byte array of a specified size.
 */
static void convert_to_array(unsigned char *array, size_t count, unsigned int v)
{
	switch (count)
	{
	case 1:
		*static_cast<unsigned char *>(array) = static_cast<unsigned char>(v);
		break;
	case 2:
		array[WORD_LO_BYTE_IDX] = static_cast<unsigned char>(v & 0xff);
		array[WORD_HI_BYTE_IDX] = static_cast<unsigned char>((v >> 8) & 0xff);
		break;
	default:
		assert(false); // Should not get here
	}
}

#endif // __COMMON_HPP__
