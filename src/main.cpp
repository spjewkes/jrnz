#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

#include "z80.hpp"
#include "ula.hpp"
#include "bus.hpp"
#include "system.hpp"
#include "debugger.hpp"
#include "options.hpp"

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;

void wait_keypress()
{
	SDL_Event event;

	while(1)
	{
		SDL_PollEvent(&event);
		if(event.type == SDL_QUIT)
		{
			return;
		}
	}
}

/**
 * @brief Main entry-point into application.
 */
int main(int argc, char **argv)
{
	std::cout << "Running jrnz..." << std::endl;

#ifdef HAVE_DISPLAY
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }

	window = SDL_CreateWindow("JRNZ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  960, 768, SDL_WINDOW_MAXIMIZED);
	if (!window)
	{
		std::cerr << "Could not create window: " << SDL_GetError() <<	std::endl;
		return EXIT_FAILURE;
    }
	renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetScale(renderer, 3.0f, 3.0f);
#else
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
	{
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#endif

	Options options(argc, argv);

	Bus mem(65536);
	Z80 state(mem, options.fast_mode);
	ULA ula(state, mem, options.fast_mode);
	Debugger debug(state, mem);

	System sys(state, ula, mem, debug);

	// Use options to set up system
	if (options.rom_on)
	{
		mem.load_rom(options.rom_file);
	}
	if (options.sna_on)
	{
		mem.load_snapshot(options.sna_file, state);
	}

	debug.set_dout(options.debug_mode);
	if (options.break_on)
	{
		debug.set_break(true, options.break_addr);
	}

	bool running = true;

	do
	{
		running = sys.clock();
	} while (running);

	std::cout << "Press any key.\n";
	wait_keypress();

#ifdef HAVE_DISPLAY
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
	
	std::cout << "Closing jrnz.\n";

	return EXIT_SUCCESS;
}
