#include <iostream>
#include <cstdlib>
#include <SDL2/SDL.h>

#include "z80.hpp"
#include "ula.hpp"
#include "bus.hpp"
#include "system.hpp"
#include "debugger.hpp"
#include "options.hpp"

// Define this to enable a window to test keyboard and display - this will need some further
// refinement before being set always
// #define HAVE_DISPLAY 1

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

	SDL_Window *window = SDL_CreateWindow("JRNZ", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
										  640, 480, SDL_WINDOW_MAXIMIZED);
	if (!window)
	{
		std::cerr << "Could not create window: " << SDL_GetError() <<	std::endl;
		return EXIT_FAILURE;
    }
#else
    if (SDL_Init(SDL_INIT_EVENTS) != 0)
	{
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
        return EXIT_FAILURE;
    }
#endif

	Options options(argc, argv);

	Bus mem(65536);
	Z80 state(mem);
	ULA ula(state, mem);
	Debugger debug(state, mem);

	System sys(state, ula, mem);

	// Use options to set up system
	if (options.rom_on)
	{
		mem.load_rom(options.rom_file);
	}
	
	debug.set_dout(options.debug_mode);
	if (options.break_on)
	{
		debug.set_break(true, options.break_addr);
	}

	bool running = true;

	do
	{
		running = debug.clock();
		running &= sys.clock(debug.is_break_enabled());
	} while (running);

#ifdef HAVE_DISPLAY
	SDL_DestroyWindow(window);
#endif
	SDL_Quit();
	
	return EXIT_SUCCESS;
}
