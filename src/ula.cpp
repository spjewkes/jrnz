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

static void	set_rendercolor(SDL_Renderer* renderer, uint8_t color, bool bright)
{
	if (bright)
	{
		switch (color)
		{
		case 0: SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); break;
		case 1: SDL_SetRenderDrawColor(renderer, 0, 0, 255, SDL_ALPHA_OPAQUE); break;
		case 2: SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); break;
		case 3: SDL_SetRenderDrawColor(renderer, 255, 0, 255, SDL_ALPHA_OPAQUE); break;
		case 4: SDL_SetRenderDrawColor(renderer, 0, 255, 0, SDL_ALPHA_OPAQUE); break;
		case 5: SDL_SetRenderDrawColor(renderer, 0, 255, 255, SDL_ALPHA_OPAQUE); break;
		case 6: SDL_SetRenderDrawColor(renderer, 255, 255, 0, SDL_ALPHA_OPAQUE); break;
		case 7: SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); break;
		}
	}
	else
	{
		switch (color)
		{
		case 0: SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE); break;
		case 1: SDL_SetRenderDrawColor(renderer, 0, 0, 215, SDL_ALPHA_OPAQUE); break;
		case 2: SDL_SetRenderDrawColor(renderer, 215, 0, 0, SDL_ALPHA_OPAQUE); break;
		case 3: SDL_SetRenderDrawColor(renderer, 215, 0, 215, SDL_ALPHA_OPAQUE); break;
		case 4: SDL_SetRenderDrawColor(renderer, 0, 215, 0, SDL_ALPHA_OPAQUE); break;
		case 5: SDL_SetRenderDrawColor(renderer, 0, 215, 215, SDL_ALPHA_OPAQUE); break;
		case 6: SDL_SetRenderDrawColor(renderer, 215, 215, 0, SDL_ALPHA_OPAQUE); break;
		case 7: SDL_SetRenderDrawColor(renderer, 215, 215, 215, SDL_ALPHA_OPAQUE); break;
		}
	}
}
#endif

void ULA::clock()
{
	UNUSED(_z80);
	UNUSED(_bus);

	switch(counter)
	{
	case 0:
		SDL_PumpEvents();

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
			set_rendercolor(renderer, _bus.port_254 & 0x7, false);
			SDL_RenderClear(renderer);

			uint8_t *data = &_bus[0x4000];
			uint8_t *col_data = &_bus[0x5800];
			for (int y=0; y<192; y++)
			{
				int new_y = 0xc0 & y;
				new_y |= (y & 0x7) << 3;
				new_y |= (y >> 3) & 0x7;
				int col_y = new_y >> 3;
				for (int x=0; x<256; x+=8)
				{
					uint8_t color = col_data[col_y * 32 + (x >> 3)];
					bool flash = get_bit(color, 7);
					bool bright = get_bit(color, 6);
					uint8_t paper_color = (color >> 3) & 0x07;
					uint8_t ink_color = color & 0x07;

					if ((new_y & 0x7) == 0 && (x & 0x7) == 0)
					{
						SDL_Rect rect = { x+32, new_y+32, 8, 8 };
						set_rendercolor(renderer, (flash & invert ? ink_color : paper_color), bright);
						SDL_RenderFillRect(renderer, &rect);
					}
					set_rendercolor(renderer, (flash & invert ? paper_color : ink_color), bright);
					
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

		frame_counter++;
		if (frame_counter % 16 == 0)
		{
			if (invert)
				invert = false;
			else
				invert = true;
		}

		uint32_t ticks = SDL_GetTicks();
		if (ticks < next_frame_ticks)
		{
			SDL_Delay(next_frame_ticks - ticks);
		}
	}

	counter++;
}
