/**
 * @brief Header containing common definitions.
 */

#ifndef __COMMON_HPP__
#define __COMMON_HPP__

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
#define HI_BYTE (1)
#define LO_BYTE (0)
#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
	defined(__LITTLE_ENDIAN__) || \
	defined(__ARMEL__) || \
	defined(__THUMBEL__) || \
	defined(__AARCH64EL__) || \
	defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)
// It's a little-endian target architecture
#define HI_BYTE (0)
#define LO_BYTE (1)
#else
#error "I don't know what architecture this is!"
#endif

/**
 * @brief Defines the operands that can be passed into an instruction handling function
 */
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

#endif // __COMMON_HPP__
