/**
 * Brief Implementation of the ULA class.
 */

#include <cstdint>
#include <SDL2/SDL.h>
#include <SDL2/SDL_surface.h>
#include <SDL2/SDL_timer.h>

#include "common.hpp"
#include "ula.hpp"

extern SDL_Window *window;
extern SDL_Renderer *renderer;

#ifdef HAVE_DISPLAY
static bool get_bit(uint8_t byte, uint8_t pos)
{
	return (byte & (1 << pos)) != 0;
}
#endif

void ULA::clock()
{
	UNUSED(_z80);
	UNUSED(_bus);

	switch(counter)
	{
	case 0:
		// Trigger interupt on Z80
		_z80.interrupt = true;
		next_frame_ticks = SDL_GetTicks() + 20;
		break;

	case 32:
		// Turn off interrupt
		_z80.interrupt = false;
		break;

	case 70000:
		//! TODO - a bit of a hack, but every 50th of a second (running at 3.5 Mhz)
	    //! reset the counter to start everything again.
		counter = UINT64_MAX; // will wrap on increment

#ifdef HAVE_DISPLAY		
		{
			// Clear screen
			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderClear(renderer);

			// Draw pixels
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			
			uint8_t *data = &_bus[0x4000];
			for (int y=0; y<192; y++)
			{
				int new_y = 0xc0 & y;
				new_y |= (y & 0x7) << 3;
				new_y |= (y >> 3) & 0x7;
				for (int x=0; x<256; x+=8)
				{
					for (int p=0; p<8; p++)
					{
						if (get_bit(*data, 7-p))
						{
							SDL_RenderDrawPoint(renderer, x+p+32 , new_y+32);
						}
					}
					data++;
				}
			}

			SDL_RenderPresent(renderer);
		}
#endif

		uint32_t ticks = SDL_GetTicks();
		if (ticks < next_frame_ticks)
		{
			SDL_Delay(next_frame_ticks - ticks);
		}
	}

	counter++;
}
