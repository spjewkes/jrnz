/**
 * Implementation of keyboard functionality.
 */

#include <SDL2/SDL.h>

#include "common.hpp"
#include "keyboard.hpp"

static bool get_bit(uint8_t byte, uint8_t pos)
{
	return (byte & (1 << pos)) != 0;
}

uint8_t get_keyboard_state(uint8_t halfrows)
{
	SDL_PumpEvents();
	const uint8_t* key_state = static_cast<const uint8_t*>(SDL_GetKeyboardState(NULL));

	uint8_t ret_keys = 0x0;

	if (key_state[SDL_SCANCODE_ESCAPE])
	{
		std::cerr << "Quiting\n";
		abort();
	}

	// half-row 0 : caps - v
	if (!get_bit(halfrows, 0))
	{
		if (key_state[SDL_SCANCODE_LSHIFT]) { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_Z])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_X])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_C])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_V])      { ret_keys |= 0x10; }
	}
	
	// half-row 1 : a - g
	if (!get_bit(halfrows, 1))
	{
		if (key_state[SDL_SCANCODE_A])      { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_S])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_D])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_F])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_G])      { ret_keys |= 0x10; }
	}
	
	// half-row 2 : q - t
	if (!get_bit(halfrows, 2))
	{
		if (key_state[SDL_SCANCODE_Q])      { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_W])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_E])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_R])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_T])      { ret_keys |= 0x10; }
	}
	
	// half-row 3 : 1 - 5
	if (!get_bit(halfrows, 3))
	{
		if (key_state[SDL_SCANCODE_1])      { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_2])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_3])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_4])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_5])      { ret_keys |= 0x10; }
	}
	
	// half-row 4 : 6 - 0
	if (!get_bit(halfrows, 4))
	{
		if (key_state[SDL_SCANCODE_0])      { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_9])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_8])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_7])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_6])      { ret_keys |= 0x10; }
	}
	
	// half-row 5 : y - p
	if (!get_bit(halfrows, 5))
	{
		if (key_state[SDL_SCANCODE_P])      { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_O])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_I])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_U])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_Y])      { ret_keys |= 0x10; }
	}
	
	// half-row 6 : h - enter
	if (!get_bit(halfrows, 6))
	{
		if (key_state[SDL_SCANCODE_RETURN]) { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_L])      { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_K])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_J])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_H])      { ret_keys |= 0x10; }
	}
	
	// half-row 7 : b - space
	if (!get_bit(halfrows, 7))
	{
		if (key_state[SDL_SCANCODE_SPACE])  { ret_keys |= 0x01; }
		if (key_state[SDL_SCANCODE_RSHIFT]) { ret_keys |= 0x02; }
		if (key_state[SDL_SCANCODE_M])      { ret_keys |= 0x04; }
		if (key_state[SDL_SCANCODE_N])      { ret_keys |= 0x08; }
		if (key_state[SDL_SCANCODE_B])      { ret_keys |= 0x10; }
	}

	// if (ret_keys != 0)
	// {
	// 	std::cout << std::hex << static_cast<unsigned int>(ret_keys) << std::endl;
	// }

	ret_keys = ~ret_keys & 0x1f;

	return ret_keys;
}
